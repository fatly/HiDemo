#include "Beautify.h"
#include "Gaussian.h"
#include "Bitmap.h"
#include "AutoPtr.h"

namespace e
{
	Beautify::Beautify(void)
	{
	}


	Beautify::~Beautify(void)
	{
	}

	void Beautify::HighContrast(uint8* dst
		, uint8* src
		, int width
		, int height
		, int bitCount)
	{
		int bpp = bitCount / 8;
		int lineBytes = WIDTHBYTES(bitCount * width);

		AutoPtr<Bitmap> blr = new Bitmap(width, height, bitCount);
		AutoPtr<Bitmap> tmp = new Bitmap(height, width, bitCount);
		Gaussian(tmp->bits, src, width, height, bitCount, 5.0f, HIGHT24);
		Gaussian(blr->bits, tmp->bits, height, width, bitCount, 5.0f, HIGHT24);

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = blr->Get(0, y);
			for (int x = 0; x < width; x++)
			{
// 				*(p1 + 0) = *(p0 + 0) - *(p1 + 0);
// 				*(p1 + 1) = *(p0 + 1) - *(p1 + 1);
// 				*(p1 + 2) = *(p0 + 2) - *(p1 + 2);

				int delta = clamp0255(*(p0 + 1) - *(p1 + 1) + 128);
				*(p1 + 0) = *(p1 + 1) = *(p1 + 2) = delta;

				p0 += bpp;
				p1 += bpp;
			}
		}

		AutoPtr<Bitmap> green = blr->Clone(1);
		green->Save("f:\\hc.bmp");

	}
}