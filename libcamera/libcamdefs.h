#ifndef __CORE_LIBCAMDEFS_H__
#define __CORE_LIBCAMDEFS_H__

#include "Defines.h"
#include <tchar.h>

namespace e
{
	//输出视频的大小
	const int KOutputVideoWidth = 480;
	const int KOutputVideoHeight = 360;
	const int KOutputVideoBitCount = 32;

	//video callback function type
	enum {
		HANDLE_TYPE_FORMAT = 0,
		HANDLE_TYPE_SAMPLE = 1,
		HANDLE_TYPE_DEVICE = 2
	};

	//define video callback function 
	typedef void(*DeviceHandle)(void* param, TCHAR* friendlyName, TCHAR* displayName);
	typedef void(*FormatHandle)(void* param, TCHAR* type, int width, int height);
	typedef void(*SampleHanlde)(void* param, uchar* buffer, int size, int width, int height, int bitCount);
}

#endif