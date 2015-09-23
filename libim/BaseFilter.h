#ifndef __LIBIM_BASEFILTER_H__
#define __LIBIM_BASEFILTER_H__
#include "Defines.h"

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//---------------------------base filter interface----------------------//
	//////////////////////////////////////////////////////////////////////////
	class BaseFilter
	{
	public:
		virtual ~BaseFilter(void){};
	public:
#ifdef INTEGER_CHANNELS
		virtual void Process(uint8* dst, uint8* src, int width, int height, int channels) = 0;
#else
		virtual void Process(float* dst, float* src, int width, int height, int channels) = 0;
#endif
	};
}

#endif