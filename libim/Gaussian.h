#ifndef __CORE_GAUSSIAN_H__
#define __CORE_GAUSSIAN_H__

#include "Defines.h"
#include "BaseFilter.h"
#define CLAMP_TO_EDGE

namespace e
{
	//gaussian mode
	enum { 
		GM_SIMPLE,
		GM_HIGH
	};

	class Gaussian : public BaseFilter
	{
	public:
		Gaussian(void);
		virtual ~Gaussian(void);
	public:
		void SetSetting(int id, void* value);
		virtual void Process(void* dst, void* src, int width, int height, int channels) override;
	protected:
		void CalcKernals(float sigma);
		void SimpleGaussian8(void* dst, void* src, int width, int height, int channels);
		void SimpleGaussian24(void* dst, void* src, int width, int height, int channels);
		void RecursiveGaussian8(void* dst, void* src, int width, int height, int channels);
		void RecursiveGaussian24(void* dst, void* src, int width, int height, int channels);
	protected:
		void SetMode(int mode);
		void SetSigma(float sigma);
		void SetConfig(int width, int height, int channels);
		typedef void(Gaussian::*GFUN)(void*, void*, int, int, int);
	protected:
		//simple or high mode
		int mode;
		//gaussian coeffs
		float sigma;
		float a0, a1, a2, a3;
		float b1, b2;
		float coefp, coefn;
		//buffer 
		void* tmp;
		int width;
		int height;
		int channels;
		GFUN fnGaussian;
	};
}

#endif