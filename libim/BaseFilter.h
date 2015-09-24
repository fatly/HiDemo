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
		virtual void SetSetting(int id, void* value) = 0;
		virtual void Process(void* dst, void* src, int width, int height, int channels) = 0;
	};

	enum {
		ID_SET_MODE = 0,
		ID_SET_RADIUS = 1,
		ID_SET_SIGMA = 2,
		ID_SET_SPACESIGMA = 3,
		ID_SET_COLORSIGMA = 4,
		ID_SET_BLURTYPE = 5
	};
}

#endif