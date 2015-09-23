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
		virtual void Process(uint8* dst, const uint8* src, int width, int height, int channels) = 0;
		virtual void Process(float* dst, const float* src, int width, int height, int channels) = 0;
	};
}

#endif