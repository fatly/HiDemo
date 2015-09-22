#include "FileIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Defines.h"

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
		uint16  bfType;
		uint32  bfSize;
		uint16  bfReserved1;
		uint16  bfReserved2;
		uint32  bfOffBits;
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

	static inline bool write(void * buffer, size_t size, FILE * fp)
	{
		return ::fwrite(buffer, 1, size, fp) == size;
	}

	static inline bool skip(size_t size, FILE * fp)
	{
		return ::fseek(fp, size, SEEK_SET) == 0;
	}

	bool FileIO::LoadBitmap(const char* fileName, char* bits, int& size, int&width, int&height, int&channels)
	{
		FILE* fp = 0;
		fopen_s(&fp, fileName, "rb");
		if (fp == 0) return false;

		BITMAPHEADER header;
		bool result = false;

		do
		{
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
			bits = (char*)malloc(size);
			assert(bits != 0);
			if (bits == 0) break;
			if (!skip(header.bfOffBits, fp))break;
			if (!read(bits, size, fp))
			{
				free(bits);
				return false;
			}
			//需要将文件中的数据上下倒转过来

			result = true;
		} while (0);

		if (fp)fclose(fp);
		return result;
	}

	bool FileIO::SaveBitmap(const char* fileName, const char* bits, int size, int width, int height, int channels)
	{

	}
}
