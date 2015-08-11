#include "Beautify.h"
#include <stdio.h>
#include <math.h>
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
	void Beautify::Screen(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		int temp = 0;
		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			if (mode == SIMPLE24 || mode == HIGHT24)
			{
				for (int x = 0; x < width; x++)
				{
					//screen op
					*(p1 + 0) = 255 - INT_MULT((255 - *(p0 + 0) * alpha0), (255 - *(p1 + 0) * alpha1), temp);
					*(p1 + 1) = 255 - INT_MULT((255 - *(p0 + 1) * alpha0), (255 - *(p1 + 1) * alpha1), temp);
					*(p1 + 2) = 255 - INT_MULT((255 - *(p0 + 2) * alpha0), (255 - *(p1 + 2) * alpha1), temp);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					//screen op
					*(p1 + 0) = 255 - INT_MULT((255 - *(p0 + 0) * alpha0), (255 - *(p1 + 0) * alpha1), temp);

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
					*(p1 + 0) = softlight(*(p1 + 0) * alpha0, *(p0 + 0) * alpha1);
					*(p1 + 1) = softlight(*(p1 + 1) * alpha0, *(p0 + 1) * alpha1);
					*(p1 + 2) = softlight(*(p1 + 2) * alpha0, *(p0 + 2) * alpha1);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*(p1 + 0) = softlight(*(p1 + 0) * alpha0, *(p0 + 0) * alpha1);

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

	void Beautify::Different(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
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
					*(p1 + 0) = clamp0255((*(p1 + 0) * alpha0 - *(p0 + 0) * alpha1));
					*(p1 + 1) = clamp0255((*(p1 + 1) * alpha0 - *(p0 + 1) * alpha1));
					*(p1 + 2) = clamp0255((*(p1 + 2) * alpha0 - *(p0 + 2) * alpha1));

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = clamp0255((*p1 * alpha0 - *p0 * alpha1));

					p0 += bpp;
					p1 += bpp;
				}
			}
		}
	}

//#define _swap(a, b) a ^= b; b ^= a; a ^= b;

	inline uint8 Beautify::overlying(uint8 a, uint8 b)
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

	void Beautify::Overlying(uint8* dst, uint8* src, int width, int height, int bitCount, int mode)
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
					*(p1 + 0) = overlying(*(p1 + 0) * alpha0, *(p0 + 0) * alpha1);
					*(p1 + 1) = overlying(*(p1 + 1) * alpha0, *(p0 + 1) * alpha1);
					*(p1 + 2) = overlying(*(p1 + 2) * alpha0, *(p0 + 2) * alpha1);

					p0 += bpp;
					p1 += bpp;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					*p1 = overlying(*(p1 + 0) * alpha0, *(p0 + 0) * alpha1);

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
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->blendIndex = 0;
		this->blendCount = 3;
		this->handles[0] = &Beautify::Overlying;
		this->handles[1] = &Beautify::SoftLight;
		this->handles[2] = &Beautify::Screen;
	}

	Beautify::~Beautify()
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
		if (key == 87) sigma += 0.02f;				//	'w'
		else if (key == 83) sigma -= 0.02f;			//	's'
		else if (key == 38) alpha0 += 0.02f;		//	'up'
		else if (key == 40) alpha0 -= 0.02f;		//	'down'
		else if (key == 37) alpha1 -= 0.02f;		//	'left'
		else if (key == 39) alpha1 += 0.02f;		//	'right'
		else if (key == 66) useBlur = !useBlur;		//	'b'
		else if (key == 70) useBlend = !useBlend;	//	'f'
		else if (key == 89) useYCbCr = !useYCbCr;	//	'y'
		else if (key == 76) blurLevel = !blurLevel;	//	'l'
		else if (key == 69)	enable = !enable;		//	'e'
		else if (key == 88) swapBlend = !swapBlend;	//	'x'
		else if (key == 65) blendIndex = (blendIndex + 1) % blendCount;	// 'a'
		else if (key == 82) Reset();

		if (sigma < 0.1f) sigma = 0.1f;
		if (alpha0 > 1.0f) alpha0 = 1.0f;
		if (alpha0 < 0.0f) alpha0 = 0.0f;
		if (alpha1 > 1.0f) alpha1 = 1.0f;
		if (alpha1 < 0.0f) alpha1 = 0.0f;
	}
}

