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
#ifdef INTEGER_CHANNELS
		virtual void Process(uint8* dst, uint8* src, int width, int height, int channels) override;
#else
		virtual void Process(float* dst, float* src, int width, int height, int channels) override;
#endif
	protected:
		void CalcParameters(float sigma);
#ifdef INTEGER_CHANNELS
		void SimpleGaussian8(uint8* dst, uint8* src, int width, int height, int channels);
		void SimpleGaussian24(uint8* dst, uint8* src, int width, int height, int channels);
		void RecursiveGaussian8(uint8* dst, uint8* src, int width, int height, int channels);
		void RecursiveGaussian24(uint8* dst, uint8* src, int width, int height, int channels);
#else
		void SimpleGaussian8(float* dst, float* src, int width, int height, int channels);
		void SimpleGaussian24(float* dst, float* src, int width, int height, int channels);
		void RecursiveGaussian8(float* dst, float* src, int width, int height, int channels);
		void RecursiveGaussian24(float* dst, float* src, int width, int height, int channels);
#endif
	};
}

#endif