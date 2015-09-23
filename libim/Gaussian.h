#ifndef __CORE_GAUSSIAN_H__
#define __CORE_GAUSSIAN_H__

#include "Defines.h"
#include "BaseFilter.h"
#define CLAMP_TO_EDGE

namespace e
{
	typedef enum { 
		GM_SIMPLE = 0, GM_HIGH = 1 
	}GaussianMode;

	class Gaussian : public BaseFilter
	{
		int mode;
		float sigma;
		float a0, a1, a2, a3;
		float b1, b2;
		float coefp, coefn;
	public:
		Gaussian(void);
		virtual ~Gaussian(void);
	public:
		void SetMode(int mode);
		void SetSigma(float sigma);
		virtual void Process(uint8* dst, const uint8* src, int width, int height, int channels) override;
		virtual void Process(float* dst, const float* src, int width, int height, int channels) override;
	protected:
		void CalcParameters(float sigma);
		template<class T> 
		void SimpleGaussian8(T* dst, const T* src, int width, int height, int channels);
		template<class T> 
		void SimpleGaussian24(T* dst, const T* src, int width, int height, int channels);
		template<class T> 
		void RecursiveGaussian8(T* dst, const T* src, int width, int height, int channels);
		template<class T> 
		void RecursiveGaussian24(T* dst, const T* src, int width, int height, int channels);
	};
}

//////////////////////////////////////////////////////////////////////////
namespace e
{
	//simple recursive gaussian
	template<class T>
	void Gaussian::SimpleGaussian8(T* dst, const T* src, int width, int height, int channels)
	{
		float a = CalcParam(sigma);
		float yp = 0.0f, xc = 0.0f, yc = 0.0f;

		int bitCount = channels * sizeof(T) * 8;
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = channels * sizeof(T);

		for (int x = 0; x < width; x++)
		{
			const T* p = src + x * bpp/sizeof(T);
			yp = *(p + 0);

			for (int y = 0; y < height; y++)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp)/sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp)/sizeof(T);

				xc = *(p0 + 0);
				yc = xc + (yp - xc) * a;
				yp = yc;
				*(p1 + 0) = T(yp);
			}

			p = src + ((height - 1) * lineBytes0 + x * bpp) / sizeof(T);
			yp = *(p + 0);

			for (int y = height - 1; y >= 0; y--)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);
				xc = *(p0 + 0);
				yc = xc + (yp - xc) * a;
				yp = yc;

				*(p1 + 0) = T((*(p1 + 0) + yc) * 0.5f);
			}
		}
	}
	template<class T>
	void Gaussian::SimpleGaussian24(T* dst, const T* src, int width, int height, int channels)
	{
		float a = CalcParam(sigma);

		float yp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float xc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float yc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		int bitCount = channels * sizeof(T) * 8;
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		for (int x = 0; x < width; x++)
		{
			const T* p = src + x * bpp/sizeof(T);
			yp[0] = *(p + 0);
			yp[1] = *(p + 1);
			yp[2] = *(p + 2);

			for (int y = 0; y < height; y++)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);

				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);

				yc[0] = xc[0] + (yp[0] - xc[0]) * a;
				yc[1] = xc[1] + (yp[1] - xc[1]) * a;
				yc[2] = xc[2] + (yp[2] - xc[2]) * a;

				yp[0] = yc[0];
				yp[1] = yc[1];
				yp[2] = yc[2];

				*(p1 + 0) = T(yp[0]);
				*(p1 + 1) = T(yp[1]);
				*(p1 + 2) = T(yp[2]);
			}

			p = src + ((height - 1) * lineBytes0 + x * bpp) / sizeof(T);
			yp[0] = *(p + 0);
			yp[1] = *(p + 1);
			yp[2] = *(p + 2);

			for (int y = height - 1; y >= 0; y--)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);
				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);

				yc[0] = xc[0] + (yp[0] - xc[0]) * a;
				yc[1] = xc[1] + (yp[1] - xc[1]) * a;
				yc[2] = xc[2] + (yp[2] - xc[2]) * a;

				*(p1 + 0) = T((*(p1 + 0) + yc[0]) * 0.5f);
				*(p1 + 1) = T((*(p1 + 1) + yc[1]) * 0.5f);
				*(p1 + 2) = T((*(p1 + 2) + yc[2]) * 0.5f);

				yp[0] = yc[0];
				yp[1] = yc[1];
				yp[2] = yc[2];
			}
		}
	}

	//recursive gaussian
	template<class T>
	void Gaussian::RecursiveGaussian8(T* dst, const T* src, int width, int height, int channels)
	{
		int bitCount = channels * sizeof(T) * 8;
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		for (int x = 0; x < width; x++)
		{
			// start forward filter pass
			float xp = 0.0f, yp = 0.0f, yb = 0.0f;

			const T* p0 = src + x * bpp/sizeof(T);
#ifdef CLAMP_TO_EDGE
			xp = *p0;
			yb = xp * coefp;
			yp = yb;
#endif
			float xc = 0.0f, yc = 0.0f;

			for (int y = 0; y < height; y++)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);

				xc = *p0;
				yc = (a0 * xc) + (a1 * xp) - (b1 * yp) - (b2 * yb);
				xp = xc;
				yb = yp;
				yp = yc;

				*p1 = T(yc);
			}

			// start reverse filter pass: ensures response is symmetrical
			float xn = 0.0f, xa = 0.0f, yn = 0.0f, ya = 0.0f;

#ifdef CLAMP_TO_EDGE
			p0 = src + ((height - 1) * lineBytes0 + x * bpp) / sizeof(T);
			xn = *p0;
			xa = xn;
			yn = xn * coefn;
			ya = yn;
#endif

			for (int y = height - 1; y >= 0; y--)
			{
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);

				xc = *p0;
				yc = (a2 * xn) + (a3 * xa) - (b1 * yn) - (b2 * ya);
				xa = xn;
				xn = xc;
				ya = yn;
				yn = yc;

				*p1 += T(yc);
			}
		}
	}

	template<class T>
	void Gaussian::RecursiveGaussian24(T* dst, const T* src, int width, int height, int channels)
	{
		int bitCount = channels * sizeof(T) * 8;
		int lineBytes0 = WIDTHBYTES(bitCount * width);
		int lineBytes1 = WIDTHBYTES(bitCount * height);
		int bpp = bitCount / 8;

		for (int x = 0; x < width; x++)
		{
			// start forward filter pass
			float xp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous input
			float yp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous output
			float yb[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // previous output by 2

			const T* p0 = src + x * bpp/sizeof(T);
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
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);

				xc[0] = *(p0 + 0);
				xc[1] = *(p0 + 1);
				xc[2] = *(p0 + 2);
				//xc[3] = *(p0 + 3);

				yc[0] = (a0 * xc[0]) + (a1 * xp[0]) - (b1 * yp[0]) - (b2 * yb[0]);
				yc[1] = (a0 * xc[1]) + (a1 * xp[1]) - (b1 * yp[1]) - (b2 * yb[1]);
				yc[2] = (a0 * xc[2]) + (a1 * xp[2]) - (b1 * yp[2]) - (b2 * yb[2]);
				//yc[3] = (a0 * xc[3]) + (a1 * xp[3]) - (b1 * yp[3]) - (b2 * yb[3]);

				*(p1 + 0) = T(yc[0]);
				*(p1 + 1) = T(yc[1]);
				*(p1 + 2) = T(yc[2]);
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
			p0 = src + ((height - 1) * lineBytes0 + x * bpp) / sizeof(T);
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
				const T* p0 = src + (y * lineBytes0 + x * bpp) / sizeof(T);
				T* p1 = dst + (x * lineBytes1 + y * bpp) / sizeof(T);

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

				*(p1 + 0) += T(yc[0]);
				*(p1 + 1) += T(yc[1]);
				*(p1 + 2) += T(yc[2]);
				//*(p1 + 3) += yc[3];
			}
		}
	}
}

#endif