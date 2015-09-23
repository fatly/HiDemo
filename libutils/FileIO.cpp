#include "private.h"
#include "FileIO.h"
#include "Defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


namespace e
{
#ifndef BI_RGB
#	define BI_RGB 0
#endif

#pragma pack(push, 1)
	typedef struct _RGBQUAD
	{
		uint8 red;
		uint8 green;
		uint8 blue;
		uint8 reserved;
	}RGBQUAD;

	typedef struct _BitmapHeader{
		uint16 bfType;
		uint32 bfSize;
		uint16 bfReserved1;
		uint16 bfReserved2;
		uint32 bfOffBits;
		uint32 biSize;
		uint32 biWidth;
		uint32 biHeight;
		uint16 biPlanes;
		uint16 biBitCount;
		uint32 biCompression;
		uint32 biSizeImage;
		uint32 biXPelsPerMeter;
		uint32 biYPelsPerMeter;
		uint32 biClrUsed;
		uint32 biClrImportant;
	}BITMAPHEADER;
#pragma pack(pop)

	static inline bool read(void * buffer, size_t size, FILE * fp)
	{
		return ::fread(buffer, 1, size, fp) == size;
	}

	static inline bool write(const void * buffer, size_t size, FILE * fp)
	{
		return ::fwrite(buffer, 1, size, fp) == size;
	}

	static inline bool skip(size_t size, FILE * fp)
	{
		return ::fseek(fp, size, SEEK_SET) == 0;
	}

	static inline void memswap(char* dst, char* src, int size)
	{
		for (int i = 0; i < size; i++)
		{
			swap(*dst++, *src++);
		}
	}

	bool FileIO::_LoadBitmap(const char* fileName, void** bits, int& size, int&width, int&height, int&channels)
	{
		assert(fileName);
		assert(bits != 0);
		if (fileName == 0) return false;

		FILE* fp = 0;
		fopen_s(&fp, fileName, "rb");
		if (fp == 0) return false;

		BITMAPHEADER header = { 0 };
		bool result = false;

		do{
			//read bitmap file header
			if (!read(&header.bfType, sizeof(header.bfType), fp)) break;
			if (header.bfType != 0x4D42) break;

			if (!read(&header.bfSize, sizeof(header.bfSize), fp)) break;
			if (!read(&header.bfReserved1, sizeof(header.bfReserved1), fp)) break;
			if (!read(&header.bfReserved2, sizeof(header.bfReserved2), fp)) break;
			if (!read(&header.bfOffBits, sizeof(header.bfOffBits), fp)) break;

			//read bitmap info header
			if (!read(&header.biSize, sizeof(header.biSize), fp)) break;
			if (!read(&header.biWidth, sizeof(header.biWidth), fp)) break;
			if (!read(&header.biHeight, sizeof(header.biHeight), fp)) break;
			if (!read(&header.biPlanes, sizeof(header.biPlanes), fp)) break;
			if (!read(&header.biBitCount, sizeof(header.biBitCount), fp)) break;
			if (!read(&header.biCompression, sizeof(header.biCompression), fp))	break;
			if (!read(&header.biSizeImage, sizeof(header.biSizeImage), fp)) break;
			if (!read(&header.biXPelsPerMeter, sizeof(header.biXPelsPerMeter), fp)) break;
			if (!read(&header.biYPelsPerMeter, sizeof(header.biYPelsPerMeter), fp))	break;
			if (!read(&header.biClrUsed, sizeof(header.biClrUsed), fp)) break;
			if (!read(&header.biClrImportant, sizeof(header.biClrImportant), fp)) break;

			if (header.biBitCount != 8 && header.biBitCount != 24 && header.biBitCount != 32)
			{
				break;
			}

			int lineBytes = WIDTHBYTES(header.biWidth * header.biBitCount);
			//有时候读取到的biSizeImage == 0，但文件不为空
			//assert(header.biHeader.biSizeImage == lineBytes * header.biHeader.biHeight);

			size = lineBytes * header.biHeight;
			*bits = (char*)realloc(*bits, size);
			assert(*bits != 0);
			if (*bits == 0) break;
			if (!skip(header.bfOffBits, fp))break;
			if (!read(*bits, size, fp))
			{
				free(*bits);
				*bits = 0;
				size = 0;
				break;
			}

			width = header.biWidth;
			height = header.biHeight;
			channels = header.biBitCount / 8;
			//需要将文件中的数据上下倒转过来
			char* s = (char*)(*bits), *d = ((char*)(*bits)) + (height - 1)* lineBytes;
			for (int i = 0; i < height / 2; i++)
			{
				memswap(d, s, lineBytes);
				s += lineBytes;
				d -= lineBytes;
			}
			result = true;
		} while (0);

		if (fp)fclose(fp);
		return result;
	}

	bool FileIO::_SaveBitmap(const char* fileName, const void* bits, int size, int width, int height, int channels)
	{
		if (fileName == 0) return false;

		FILE* fp = NULL;
		fopen_s(&fp, fileName, "wb");
		if (fp == 0) return false;

		bool result = false;
		int bitCount = channels * 8;
		do{
			int lineBytes = WIDTHBYTES(width * bitCount);
			int paletteSize = (1 << bitCount) * sizeof(RGBQUAD);
			int imageSize = lineBytes * height;

			BITMAPHEADER header = { 0 };
			header.bfType = 0x4D42;
			header.bfSize = 0;
			header.bfReserved1 = 0;
			header.bfReserved2 = 0;
			header.bfOffBits = 0;

			if (bitCount == 8)
			{
				header.bfOffBits = 54 + paletteSize;
				header.bfSize = 54 + paletteSize + imageSize;
			}
			else if (bitCount == 24 || bitCount == 32)
			{
				header.bfOffBits = 54;
				header.bfSize = 54 + imageSize;
			}

			header.biSize = 40;
			header.biWidth = width;
			header.biHeight = height;
			header.biPlanes = 1;
			header.biBitCount = bitCount;
			header.biCompression = BI_RGB;
			header.biSizeImage = imageSize;
			header.biXPelsPerMeter = 3780;
			header.biYPelsPerMeter = 3780;
			header.biClrUsed = 0;
			header.biClrImportant = 0;

			if (!write(&header, sizeof(header), fp)) break;

			if (bitCount == 8)
			{
				RGBQUAD* rgbQuad = new RGBQUAD[1 << bitCount];
				assert(rgbQuad != 0);

				for (int i = 0; i < (1 << bitCount); i++)
				{
					rgbQuad[i].red = i;
					rgbQuad[i].green = i;
					rgbQuad[i].blue = i;
				}

				if (!write(rgbQuad, sizeof(RGBQUAD) * (1 << bitCount), fp))
				{
					delete[] rgbQuad;
					break;
				}

				delete[] rgbQuad;
			}
			//需要将文件中的数据上下倒转过来
			const char* p = (const char*)bits + (height - 1) * lineBytes;
			for (int y = 0; y < height; y++)
			{
				if (!write(p, lineBytes, fp)) goto _error;
				p -= lineBytes;
			}

			result = true;
		} while (0);

	_error:
		if (fp) fclose(fp);
		return result;
	}
}
