#include "ColorSpace.h"

namespace e
{
	void ColorSpace::BGRA2YCC(uint8* dst, uint8* src, int width, int height, int bitCount)
	{
		int bpp = bitCount / 8;
		int lineBytes = WIDTHBYTES(width * bitCount);

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			for (int x = 0; x < width; x++)
			{
				uint8 B = *(p0 + 0);
				uint8 G = *(p0 + 1);
				uint8 R = *(p0 + 2);
				int Y  = 0.299 * R + 0.587 * G + 0.114 * B;
				int Cb = -0.1687 * R - 0.3313 * G + 0.5 * B + 128;
				int Cr = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;
				*(p1 + 0) = clamp0255(Y);
				*(p1 + 1) = clamp0255(Cb);
				*(p1 + 2) = clamp0255(Cr);

				p0 += bpp;
				p1 += bpp;
			}
		}
	}

	void ColorSpace::YCC2BGRA(uint8* dst, uint8* src, int width, int height, int bitCount)
	{
		int bpp = bitCount / 8;
		int lineBytes = WIDTHBYTES(width * bitCount);

		for (int y = 0; y < height; y++)
		{
			uint8* p0 = src + y * lineBytes;
			uint8* p1 = dst + y * lineBytes;

			for (int x = 0; x < width; x++)
			{
				uint8 Y = *(p0 + 0);
				uint8 Cb = *(p0 + 1);
				uint8 Cr = *(p0 + 2);

				int R = Y + 1.402 * (Cr - 128);
				int	G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128);
				int	B = Y + 1.772 * (Cb - 128);

				*(p1 + 0) = clamp0255(B);
				*(p1 + 1) = clamp0255(G);
				*(p1 + 2) = clamp0255(R);

				p0 += bpp;
				p1 += bpp;
			}
		}
	}
}

