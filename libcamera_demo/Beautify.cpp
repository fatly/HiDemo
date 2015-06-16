#include "Beautify.h"
#include <stdio.h>
#include <math.h>
#include "Gaussian.h"
#include "libutils.h"

#pragma comment(lib, "libutils.lib")

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))

namespace e
{
	//1.0 - (1.0 - a) * (1.0 - b);	
	void Beautify::Screen(uint8* dst, uint8* src, int width, int height, int bitCount)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		int temp = 0;
		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

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
	}

	inline uint8 softlight(uint8 a, uint8 b)
	{
		if (b <= 128)
		{
			return (a * b) / 128 + (a / 255) * (a / 255) * (255 - 2 * b);
		}
		else
		{
			return a * (255 - b) / 128 + sqrt((float)a / 255.0f) * (2 * b - 255);
		}
	}

	void Beautify::SoftLight(uint8* dst, uint8* src, int width, int height, int bitCount)
	{
		int lineBytes = WIDTHBYTES(bitCount * width);
		int bpp = bitCount / 8;

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			for (int x = 0; x < width; x++)
			{
				*(p1 + 0) = softlight(*(p1 + 0) * alpha0, *(p0 + 0) * alpha1);
				*(p1 + 1) = softlight(*(p1 + 1) * alpha0, *(p0 + 1) * alpha1);
				*(p1 + 2) = softlight(*(p1 + 2) * alpha0, *(p0 + 2) * alpha1);

				p0 += bpp;
				p1 += bpp;
			}
		}
	}

	Beautify::Beautify(int width, int height)
	{
		this->sigma = 5.0f;
		this->width = width;
		this->height = height;
		this->enable = true;
		this->src = new uint8[width * height * 4];
		this->tmp = new uint8[width * height * 4];
		this->alpha0 = 1.0f;
		this->alpha1 = 1.0f;
		this->mode = 0;
		this->blendHandle = &Beautify::Screen;
	}


	Beautify::~Beautify()
	{
		if (src)
		{
			delete[] src;
		}

		if (tmp)
		{
			delete[] tmp;
		}
	}

	void Beautify::Process(uint8* buffer, int width, int height, int bitCount)
	{
		int bytes = WIDTHBYTES(width * bitCount) * height;
		memcpy(src, buffer, bytes);

		Gaussian(tmp, src, width, height, bitCount, sigma, mode);
		Gaussian(src, tmp, height, width, bitCount, sigma, mode);

		if (enable)
		{
			(this->*blendHandle)(buffer, src, width, height, bitCount);
		}
		else
		{
			memcpy(buffer, src, bytes);
		}
	}

	void Beautify::KeyDown(int key)
	{
		if (key == 65) sigma += 0.1f;//a
		else if (key == 68) sigma -= 0.1f;//d
		else if (key == 70) enable = !enable;
		else if (key == 38) alpha0 += 0.1f;
		else if (key == 40) alpha0 -= 0.1f;
		else if (key == 37) alpha1 += 0.1f;
		else if (key == 39) alpha1 -= 0.1f;
		else if (key == 77) mode = !mode;
		else if (key == 78)
		{
			if (blendHandle == &Beautify::Screen)
			{
				blendHandle = &Beautify::SoftLight;
			}
			else
			{
				blendHandle = &Beautify::Screen;
			}
		}

		if (sigma < 0.1f) sigma = 0.1f;
		if (alpha0 > 1.0f) alpha0 = 1.0f;
		if (alpha0 < 0.0f) alpha0 = 0.0f;
		if (alpha1 > 1.0f) alpha1 = 1.0f;
		if (alpha1 < 0.0f) alpha1 = 0.0f;
	}
}

