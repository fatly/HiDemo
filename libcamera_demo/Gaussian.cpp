#include "Gaussian.h"
#include <math.h>

#pragma warning(disable:4244)

#define CLAMP_TO_EDGE

namespace e
{
	inline float CalcParam(float sigma)
	{
		return exp(-(1.695f / sigma));
	}
	//simple recursive gaussian
	void Gaussian0(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma)
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

	struct GaussParams
	{
		float sigma;
		float alpha;
		float ema;
		float ema2;
		float a0;
		float a1;
		float a2;
		float a3;
		float b1;
		float b2;
		float coefp;
		float coefn;
	};
	//recursive gaussian
	void Gaussian(uint8* dst
		, uint8* src
		, int width
		, int height
		, int bitCount
		, float a0
		, float a1
		, float a2
		, float a3
		, float b1
		, float b2
		, float coefp
		, float coefn)
	{
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		for (int x = 0; x < width; x++)
		{
			// start forward filter pass
			float xp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous input
			float yp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous output
			float yb[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous output by 2

			uint8* p0 = src + x * bpp;
#ifdef CLAMP_TO_EDGE
			xp[0] = *(p0 + 0);
			xp[1] = *(p0 + 1);
			xp[2] = *(p0 + 2);
			//xp[3] = *(p0 + 3);

			for (int i = 0; i < 4; i++)
			{
				yb[i] = xp[i] * coefp;
				yp[i] = yb[i];
			}
#endif

			float xc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float yc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

			for (int y = 0; y < height; y++)
			{
				uint8* p0 = src + y * lineBytes0 + x * bpp;
				uint8* p1 = dst + x * lineBytes1 + y * bpp;
				
				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);
				//xc[3] = *(p0 + 3);

				yc[0] = (a0 * xc[0]) + (a1 * xp[0]) - (b1 * yp[0]) - (b2 * yb[0]);
				yc[1] = (a0 * xc[1]) + (a1 * xp[1]) - (b1 * yp[1]) - (b2 * yb[1]);
				yc[2] = (a0 * xc[2]) + (a1 * xp[2]) - (b1 * yp[2]) - (b2 * yb[2]);
				//yc[3] = (a0 * xc[3]) + (a1 * xp[3]) - (b1 * yp[3]) - (b2 * yb[3]);

				*(p1 + 0) = yc[0];
				*(p1 + 1) = yc[1];
				*(p1 + 2) = yc[2];
				//*(p1 + 3) = yc[3];

				for (int i = 0; i < 4; i++)
				{
					xp[i] = xc[i];
					yb[i] = yp[i];
					yp[i] = yc[i];
				}
			}

			// start reverse filter pass: ensures response is symmetrical
			float xn[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float xa[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float yn[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float ya[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

#ifdef CLAMP_TO_EDGE
			p0 = src + (height - 1) * lineBytes0 + x * bpp;
			xn[0] = *(p0 + 0);
			xn[1] = *(p0 + 1);
			xn[2] = *(p0 + 2);
			//xn[3] = *(p0 + 3);

			for (int i = 0; i < 4; i++)
			{
				xa[i] = xn[i];
				yn[i] = xn[i] * coefn;
				ya[i] = yn[i];
			}
#endif

			for (int y = height - 1; y >= 0; y--)
			{
				uint8* p0 = src + y * lineBytes0 + x * bpp;
				uint8* p1 = dst + x * lineBytes1 + y * bpp;

				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);
				//xc[3] = *(p0 + 3);

				yc[0] = (a2 * xn[0]) + (a3 * xa[0]) - (b1 * yn[0]) - (b2 * ya[0]);
				yc[1] = (a2 * xn[1]) + (a3 * xa[1]) - (b1 * yn[1]) - (b2 * ya[1]);
				yc[2] = (a2 * xn[2]) + (a3 * xa[2]) - (b1 * yn[2]) - (b2 * ya[2]);
				//yc[3] = (a2 * xn[3]) + (a3 * xa[3]) - (b1 * yn[3]) - (b2 * ya[3]);

				for (int i = 0; i < 4; i++)
				{
					xa[i] = xn[i];
					xn[i] = xc[i];
					ya[i] = yn[i];
					yn[i] = yc[i];
				}

				*(p1 + 0) += yc[0];
				*(p1 + 1) += yc[1];
				*(p1 + 2) += yc[2];
				//*(p1 + 3) += yc[3];
			}
		}
	}

	inline void CalcParam(float sigma, GaussParams & param)
	{
		// pre-compute filter coefficients
		param.sigma = sigma; // note: fSigma is range-checked and clamped >= 0.1f upstream
		param.alpha = 1.695f / param.sigma;
		param.ema = exp(-param.alpha);
		param.ema2 = exp(-2.0f * param.alpha);
		param.b1 = -2.0f * param.ema;
		param.b2 = param.ema2;
		param.a0 = 0.0f;
		param.a1 = 0.0f;
		param.a2 = 0.0f;
		param.a3 = 0.0f;
		param.coefp = 0.0f;
		param.coefn = 0.0f;

		float k = (1.0f - param.ema)*(1.0f - param.ema) / (1.0f + (2.0f * param.alpha * param.ema) - param.ema2);
		param.a0 = k;
		param.a1 = k * (param.alpha - 1.0f) * param.ema;
		param.a2 = k * (param.alpha + 1.0f) * param.ema;
		param.a3 = -k * param.ema2;

		param.coefp = (param.a0 + param.a1) / (1.0f + param.b1 + param.b2);
		param.coefn = (param.a2 + param.a3) / (1.0f + param.b1 + param.b2);
	}

	void Gaussian1(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma)
	{
		GaussParams param;

		CalcParam(sigma, param);

		Gaussian(dst
			, src
			, width
			, height
			, bitCount
			, param.a0
			, param.a1
			, param.a2
			, param.a3
			, param.b1
			, param.b2
			, param.coefp
			, param.coefn);
	}

	void Gaussian(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode)
	{
		if (mode == 0)
		{
			Gaussian0(dst, src, width, height, bitCount, sigma);
		}
		else if (mode == 1)
		{
			Gaussian1(dst, src, width, height, bitCount, sigma);
		}
	}
}


