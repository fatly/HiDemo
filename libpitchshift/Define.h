#ifndef __LIBPITCH_SHIFT_DEFINE_H__
#define __LIBPITCH_SHIFT_DEFINE_H__
#include "config.h"
#include <stdexcept>

namespace e
{

#if defined(INTEGER_SAMPLES) 
	typedef short sample_t;
	typedef long lsample_t;
#else
	typedef float sample_t;
	typedef double lsample_t;
#endif

	// 不定长整形
	typedef char int8;
	typedef short int16;
	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned short ushort;
	typedef unsigned char uchar;
	//	typedef unsigned char byte;

	// 定长整型
#if defined(_WIN64) || defined(__x86_64__)
#define E_64
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef long long intx;
	typedef unsigned long long uintx;
	typedef unsigned long long ulongptr;
#else
#define E_32
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef int intx;
	typedef unsigned int uintx;
	typedef unsigned long ulongptr;
#endif

	// 字符
#ifdef UNICODE
	typedef wchar_t Char;
	typedef wchar_t tchar;
	//#   define _T(s) L##s
#else
	typedef char Char;
	typedef char tchar;
	//#   define _T(s) s
#endif

#define PI        3.141592655357989
#define TWOPI    (2 * PI)

#define E_THROW(x) {throw std::runtime_error(x);}

#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)<(b)?(b):(a))
#define clamp(x, a, b) max((a), min((x), (b)))
#define clamp16(x) clamp((x),(-32768), (32767))

#define ALIGN_POINTER_16(x)  (((ulongptr)(x) + 15) & ~(ulongptr)15)

}

#endif
