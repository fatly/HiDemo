#ifndef __LIBIM_BLURFILTER_H__
#define __LIBIM_BLURFILTER_H__
#include "Defines.h"
#include "Gaussian.h"

namespace e
{
	class BlurFilter
	{
	public:
		BlurFilter(void);
		BlurFilter(float sigma);
		virtual ~BlurFilter(void);
	public:
		void SetSize(int width, int height);
		void SetSigma(float sigma);
#ifdef INTEGER_CHANNELS
		void Process(uint8* dst, uint8* src, int width, int height, int channels);
#else
		void Process(float* dst, float* src, int width, int height, int channels);
#endif
	protected:
		int width;
		int height;
		uint8* tmp;
		Gaussian* filter;
	};
}

#endif