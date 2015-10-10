#ifndef __CORE_LIBCAMDEFS_H__
#define __CORE_LIBCAMDEFS_H__

#include "Defines.h"
#include <tchar.h>

namespace e
{
	enum {
		HANDLE_TYPE_FORMAT = 0,
		HANDLE_TYPE_SAMPLE = 1,
		HANDLE_TYPE_DEVICE = 2
	};

	typedef void(*FormatHandle)(void* param, TCHAR* type, int width, int height);
	typedef void(*SampleHanlde)(void* param, uchar* buffer, int width, int height, int bitCount);
	typedef void(*DeviceHandle)(void* param, TCHAR* friendlyName, TCHAR* displayName);
}

#endif