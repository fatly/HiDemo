#include "Beautify.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <memory.h>
#include "Gaussian.h"
#include "libutils.h"
#include "ColorSpace.h"

#ifdef _DEBUG
#pragma comment(lib, "libutilsd.lib")
#else
#pragma comment(lib, "libutils.lib")
#endif

#pragma warning(disable:4244)

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))

namespace e
{
	//1.0 - (1.0 - a) * (1.0 - b);	
	inline uint8 Beautify::screen(uint8 a, uint8 b)
	{
		return 255 - (255 - a) * (255 - b) / 255;
	}

	void Beautify::Screen(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
// 					*(p1 + 0) = screen(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
// 					*(p1 + 1) = screen(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
// 					*(p1 + 2) = screen(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

					*(p1 + 0) = softlight(*(p1 + 0), screen(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
					*(p1 + 1) = softlight(*(p1 + 1), screen(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
					*(p1 + 2) = softlight(*(p1 + 2), screen(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

 					#define BLEND(a, b) blend[(a<<8 | b) & 0xffff]
// 
// 					*(p1 + 0) = BLEND(*(p1 + 0), *(p0 + 0));
// 					*(p1 + 1) = BLEND(*(p1 + 1), *(p0 + 1));
// 					*(p1 + 2) = BLEND(*(p1 + 2), *(p0 + 1));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					//screen op
					//*p1 = softlight(*p1, screen(*p1 * alpha1, *p0 * alpha0));
					*p1 = BLEND(*p1, *p0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	inline uint8 Beautify::softlight(uint8 a, uint8 b)
	{
		if (this->swapBlend) swap(a, b);

		if (b <= 128)
		{
			return (a * b) / 128 + (a / 255) * (a / 255) * (255 - 2 * b);
		}
		else
		{
			return a * (255 - b) / 128 + sqrt((float)a / 255.0f) * (2 * b - 255);
		}
	}

	void Beautify::SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = softlight(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
					*(p1 + 1) = softlight(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
					*(p1 + 2) = softlight(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);

// 					*(p1 + 0) = overlying(*(p1 + 0), softlight(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
// 					*(p1 + 1) = overlying(*(p1 + 1), softlight(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
// 					*(p1 + 2) = overlying(*(p1 + 2), softlight(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = softlight(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

//#define _swap(a, b) a ^= b; b ^= a; a ^= b;

	inline uint8 Beautify::overlay(uint8 a, uint8 b)
	{
		if (this->swapBlend) swap(a, b);

		if (a < 128)
		{
			return a * b / 128;
		}
		else
		{
			return 255 - (255 - a) * (255 - b) / 128;
		}
	}

	void Beautify::Overlay(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
// 					*(p1 + 0) = overlay(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0);
// 					*(p1 + 1) = overlay(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0);
// 					*(p1 + 2) = overlay(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0);
					*(p1 + 0) = softlight(*(p1 + 0), overlay(*(p1 + 0) * alpha1, *(p0 + 0) * alpha0));
					*(p1 + 1) = softlight(*(p1 + 1), overlay(*(p1 + 1) * alpha1, *(p0 + 1) * alpha0));
					*(p1 + 2) = softlight(*(p1 + 2), overlay(*(p1 + 2) * alpha1, *(p0 + 2) * alpha0));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = overlay(*p1 * alpha1, *p0 * alpha0);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	Beautify::Beautify(int width, int height)
	{
		this->sigma = 5.0f;
		this->width = width;
		this->height = height;
		this->enable = true;
		this->useBlur = true;
		this->useBlend = true;
		this->useYCbCr = false;
		this->swapBlend = false;
		this->blurLevel = true;
		this->blr = new uint8[width * height * 4];
		this->tmp = new uint8[width * height * 4];
		this->yuv = new uint8[width * height * 4];
		this->blend = new uint8[256 * 256];
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->blendIndex = 0;
		this->blendCount = 3;
		this->handles[0] = &Beautify::Overlay;
		this->handles[1] = &Beautify::SoftLight;
		this->handles[2] = &Beautify::Screen;

		Preset();
	}

	Beautify::~Beautify(void)
	{
		if (blr)
		{
			delete[] blr;
		}

		if (tmp)
		{
			delete[] tmp;
		}

		if (yuv)
		{
			delete[] yuv;
		}

		if (blend)
		{
			delete[] blend;
		}
	}

	void Beautify::Reset(void)
	{
		this->sigma = 3.0f;
		this->enable = true;
		this->useBlur = true;
		this->useBlend = true;
		this->useYCbCr = false;
		this->swapBlend = false;
		this->blurLevel = true;
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->blendIndex = 0;
	}

	void Beautify::Preset(void)
	{
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < 256; j++)
			{
				uint8 value = softlight(i, screen(i, j));
				int index = ((i << 8) | j) & 0xffff;
				blend[index] = value;
			}
		}
	}

	void Beautify::Process(uint8* buffer, int width, int height, int bitCount)
	{
		if (!enable) return;

		int bytes = WIDTHBYTES(width * bitCount) * height;

		if (!useYCbCr)
		{
			memcpy(blr, buffer, bytes);

			if (useBlur)
			{
				Gaussian(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT24 : SIMPLE24);
				Gaussian(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT24 : SIMPLE24);
			}

			if (useBlend)
			{
				(this->*handles[blendIndex])(buffer, blr, width, height, bitCount, HIGHT24);
			}
			else
			{
				memcpy(buffer, blr, bytes);
			}
		}
		else
		{
			ColorSpace::BGRA2YCC(yuv, buffer, width, height, bitCount);

			memcpy(blr, yuv, bytes);

			if (useBlur)
			{
				Gaussian(tmp, blr, width, height, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
				Gaussian(blr, tmp, height, width, bitCount, sigma, blurLevel ? HIGHT8 : SIMPLE8);
			}

			if (useBlend)
			{
				(this->*handles[blendIndex])(yuv, blr, width, height, bitCount, HIGHT8);
			}
			else
			{
				memcpy(yuv, blr, bytes);
			}

			ColorSpace::YCC2BGRA(buffer, yuv, width, height, bitCount);
		}
	}

	void Beautify::KeyDown(int key)
	{
		if (key == 65) sigma -= 0.02f;				//	'a'
		else if (key == 68) sigma += 0.02f;			//	'd'
		else if (key == 37) alpha0 -= 0.02f;		//	'left'
		else if (key == 39) alpha0 += 0.02f;		//	'right'
		else if (key == 40) alpha1 -= 0.02f;		//	'down'
		else if (key == 38) alpha1 += 0.02f;		//	'up'
		else if (key == 66) useBlur = !useBlur;		//	'b'
		else if (key == 70) useBlend = !useBlend;	//	'f'
		else if (key == 89) useYCbCr = !useYCbCr;	//	'y'
		else if (key == 76) blurLevel = !blurLevel;	//	'l'
		else if (key == 69)	enable = !enable;		//	'e'
		else if (key == 88) swapBlend = !swapBlend;	//	'x'
		else if (key == 87) blendIndex = (blendIndex + 1) % blendCount;	// 'w'
		else if (key == 82) Reset();

		if (sigma < 0.1f) sigma = 0.1f;
		if (alpha0 > 1.0f) alpha0 = 1.0f;
		if (alpha0 < 0.0f) alpha0 = 0.0f;
		if (alpha1 > 1.0f) alpha1 = 1.0f;
		if (alpha1 < 0.0f) alpha1 = 0.0f;
	}

	String Beautify::GetParamText()
	{
		String result, line;
		TCHAR szBlend[3][256] = { TEXT("overlay"), TEXT("softlight"), TEXT("screen") };
		line = String::Format(TEXT("sigma = %f\n"), sigma);
		result.Append(line);
		line = String::Format(TEXT("alpha0 = %f\n"), alpha0);
		result.Append(line);
		line = String::Format(TEXT("alpha1 = %f\n"), alpha1);
		result.Append(line);
		line = String::Format(TEXT("useBlur = %s\n"), useBlur ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("useBlend = %s\n"), useBlend ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("useYCbCr = %s\n"), useYCbCr ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("blurLevel = %s\n"), blurLevel ? TEXT("hight") : TEXT("simple"));
		result.Append(line);
		line = String::Format(TEXT("swapBlend = %s\n"), swapBlend ? TEXT("true") : TEXT("false"));
		result.Append(line);
		line = String::Format(TEXT("blendMode = %s\n"), szBlend[blendIndex]);
		result.Append(line);
		line = String::Format(TEXT("enable = %s\n"), enable ? TEXT("true") : TEXT("false"));
		result.Append(line);

		return result;
	}
}

