#ifndef __CORE_HANDLEDEFS_H__
#define __CORE_HANDLEDEFS_H__

#include "Defines.h"
#include <tchar.h>

namespace e
{
	enum {
		HANDLE_TYPE_FORMAT = 0,
		HANDLE_TYPE_SAMPLE = 1
	};

	typedef void(*FormatHandle)(void* param, TCHAR* type, int width, int height);
	typedef void(*SampleHanlde)(void* param, uchar* buffer, int width, int height, int bitCount);
}

#endif