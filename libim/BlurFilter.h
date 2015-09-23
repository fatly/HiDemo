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

		template<class T>
		void Process(T* dst, const T* src, int width, int height, int channels)
		{
			if (this->width != width || this->height != height)
			{
				SetSize(width, height);
			}
			//gaussian blur
			filter->Process(tmp, src, width, height, channels);
			filter->Process(dst, tmp, height, width, channels);
		}
	protected:
		int width;
		int height;
		uint8* tmp;
		Gaussian* filter;
	};
}

#endif