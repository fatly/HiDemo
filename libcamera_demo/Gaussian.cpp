#include "Gaussian.h"
#include <math.h>

namespace e
{
	inline float CalcParam(float sigma)
	{
		return exp(-(1.695f / sigma));
	}

	void Gaussian(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma)
	{
		float a = CalcParam(sigma);

		float yp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float xc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float yc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		for (int x = 0; x < width; x++)
		{
			uint8* p = src + x * bpp;
			yp[0] = *(p + 0);
			yp[1] = *(p + 1);
			yp[2] = *(p + 2);

			for (int y = 0; y < height; y++)
			{
				uint8* p0 = src + y * lineBytes0 + x * bpp;
				uint8* p1 = dst + x * lineBytes1 + y * bpp;

				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);

				yc[0] = xc[0] + (yp[0] - xc[0]) * a;
				yc[1] = xc[1] + (yp[1] - xc[1]) * a;
				yc[2] = xc[2] + (yp[2] - xc[2]) * a;

				*(p1 + 0) = yp[0] = yc[0];
				*(p1 + 1) = yp[1] = yc[1];
				*(p1 + 2) = yp[2] = yc[2];
			}

			p = src + (height - 1) * lineBytes0 + x * bpp;
			yp[0] = *(p + 0);
			yp[1] = *(p + 1);
			yp[2] = *(p + 2);

			for (int y = height - 1; y >= 0; y--)
			{
				uint8* p0 = src + y * lineBytes0 + x * bpp;
				uint8* p1 = dst + x * lineBytes1 + y * bpp;
				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);

				yc[0] = xc[0] + (yp[0] - xc[0]) * a;
				yc[1] = xc[1] + (yp[1] - xc[1]) * a;
				yc[2] = xc[2] + (yp[2] - xc[2]) * a;

				*(p1 + 0) = (*(p1 + 0) + yc[0]) * 0.5f;
				*(p1 + 1) = (*(p1 + 1) + yc[1]) * 0.5f;
				*(p1 + 2) = (*(p1 + 2) + yc[2]) * 0.5f;

				yp[0] = yc[0];
				yp[1] = yc[1];
				yp[2] = yc[2];
			}
		}
	}
}


