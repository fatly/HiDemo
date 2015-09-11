#include "FastBlur.h"
#include <emmintrin.h>
#include <math.h>

namespace e
{
#define CLAMP_TO_EDGE

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
	inline float CalcParam(float sigma)
	{
		return exp(-(1.695f / sigma));
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

	void FastBlur32A(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma)
	{
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		__m128 a = _mm_set_ps1(CalcParam(sigma));
		__m128 yp = _mm_setzero_ps();
		__m128 xc = _mm_setzero_ps();
		__m128 yc = _mm_setzero_ps();
		__m128 tmp = _mm_setzero_ps();
		__m128 hal = _mm_set_ps1(0.5f);

		for (int x = 0; x < width; x++)
		{
			uint8* s = src + x * bpp;
			yp = _mm_set_ps(s[3], s[2], s[1], s[0]);

			for (int y = 0; y < height; y++)
			{
				uint8* p = src + y * lineBytes0 + x * bpp;
				uint8* d = dst + x * lineBytes1 + y * bpp;

				xc = _mm_set_ps(p[3], p[2], p[1], p[0]);
				yc = _mm_add_ps(xc, _mm_mul_ps(_mm_sub_ps(yp, xc), a));
				_mm_store_ps(yp.m128_f32, yc);

				d[0] = yc.m128_f32[0];
				d[1] = yc.m128_f32[1];
				d[2] = yc.m128_f32[2];
				d[3] = yc.m128_f32[3];
			}

			s = src + (height - 1) * lineBytes0 + x * bpp;
			yp = _mm_set_ps(s[3], s[2], s[1], s[0]);

			for (int y = height - 1; y >= 0; y--)
			{
				uint8* p = src + y * lineBytes0 + x * bpp;
				uint8* d = dst + x * lineBytes1 + y * bpp;

				xc = _mm_set_ps(p[3], p[2], p[1], p[0]);
				yc = _mm_add_ps(xc, _mm_mul_ps(_mm_sub_ps(yp, xc), a));
				_mm_store_ps(yp.m128_f32, yc);

				tmp = _mm_mul_ps(_mm_add_ps(yc, xc), hal);

				d[0] = tmp.m128_f32[0];
				d[1] = tmp.m128_f32[1];
				d[2] = tmp.m128_f32[2];
				d[3] = tmp.m128_f32[3];
			}
		}
	}

	void FastBlur32B(uint8* dst
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

		__m128 _a0 = _mm_set_ps1(a0);
		__m128 _a1 = _mm_set_ps1(a1);
		__m128 _a2 = _mm_set_ps1(a2);
		__m128 _a3 = _mm_set_ps1(a3);
		__m128 _b1 = _mm_set_ps1(b1);
		__m128 _b2 = _mm_set_ps1(b2);
		__m128 _coefp = _mm_set_ps1(coefp);
		__m128 _coefn = _mm_set_ps1(coefn);

		for (int x = 0; x < width; x++)
		{
			uint8* s = src + x * bpp;

			__m128 xp = _mm_setzero_ps();
			__m128 yp = _mm_setzero_ps();
			__m128 yb = _mm_setzero_ps();
			__m128 xc = _mm_setzero_ps();
			__m128 yc = _mm_setzero_ps();

#ifdef CLAMP_TO_EDGE
			xp = _mm_set_ps(s[3], s[2], s[1], s[0]);
			_mm_store_ps(yb.m128_f32, _mm_mul_ps(xp, _coefp));
			_mm_store_ps(yp.m128_f32, yb);
#endif

			for (int y = 0; y < height; y++)
			{
				uint8* p = src + y * lineBytes0 + x * bpp;
				uint8* d = dst + x * lineBytes1 + y * bpp;

				xc = _mm_set_ps(p[3], p[2], p[1], p[0]);
				yc = _mm_add_ps(_mm_mul_ps(_a0, xc), _mm_mul_ps(_a1, xp));
				yc = _mm_sub_ps(yc, _mm_mul_ps(_b1, yp));
				yc = _mm_sub_ps(yc, _mm_mul_ps(_b2, yb));

				d[0] = yc.m128_f32[0];
				d[1] = yc.m128_f32[1];
				d[2] = yc.m128_f32[2];
				d[3] = yc.m128_f32[3];

				_mm_store_ps(xp.m128_f32, xc);
				_mm_store_ps(yb.m128_f32, yp);
				_mm_store_ps(yp.m128_f32, yc);
			}

			__m128 xn = _mm_setzero_ps();
			__m128 xa = _mm_setzero_ps();
			__m128 yn = _mm_setzero_ps();
			__m128 ya = _mm_setzero_ps();

#ifdef CLAMP_TO_EDGE
			s = src + (height - 1) * lineBytes0 + x * bpp;

			xn = _mm_set_ps(s[3], s[2], s[1], s[0]);
			_mm_store_ps(xa.m128_f32, xn);
			yn = _mm_mul_ps(xn, _coefn);
			_mm_store_ps(ya.m128_f32, yn);
#endif

			for (int y = height - 1; y >= 0; y--)
			{
				uint8* p = src + y * lineBytes0 + x * bpp;
				uint8* d = dst + x * lineBytes1 + y * bpp;

				xc = _mm_set_ps(p[3], p[2], p[1], p[0]);
				yc = _mm_add_ps(_mm_mul_ps(_a2, xn), _mm_mul_ps(_a3, xa));
				yc = _mm_sub_ps(yc, _mm_mul_ps(_b1, yn));
				yc = _mm_sub_ps(yc, _mm_mul_ps(_b2, ya));

				_mm_store_ps(xa.m128_f32, xn);
				_mm_store_ps(xn.m128_f32, xc);
				_mm_store_ps(ya.m128_f32, yn);
				_mm_store_ps(yn.m128_f32, yc);

				d[0] += yc.m128_f32[0];
				d[1] += yc.m128_f32[1];
				d[2] += yc.m128_f32[2];
				d[3] += yc.m128_f32[3];
			}
		}
	}

	void FastBlur32C(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode)
	{

	}

	void FastBlur(uint8* dst, uint8* src, int width, int height, int bitCount, float sigma, int mode)
	{
		if (mode == 1)
		{
			GaussParams param;
			CalcParam(sigma, param);

			FastBlur32B(dst
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
		else
		{
			FastBlur32A(dst, src, width, height, bitCount, sigma);
		}
	}
}