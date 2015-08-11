#ifndef __CORE_DEFINES_H__
#define __CORE_DEFINES_H__

namespace e
{
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
#else
#define E_32
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef int intx;
	typedef unsigned int uintx;
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

	// 其他数值类型
	typedef float	real;
	typedef uint32	RGBA;

	// 其他定义
#ifdef __GNUC__
#	define override
#endif

#ifndef PI
#define PI 3.141592654
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) ((a)<(b)?(b):(a))
#endif

#ifndef CLAMP
#define CLAMP(x, a, b) MAX((a), MIN((x), (b)))
#endif

#ifndef ROUND
#define ROUND(x) ((int)((x) + ((x)>0?0.5:-0.5)))
#endif

#ifndef	WIDTHBYTES
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(x) if ((x) != 0){free(x); (x) = 0;}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if ((x) != 0){delete (x); (x) = 0;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if ((x) != 0){delete[] (x); (x) = 0;}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if ((x) != 0){(x)->Release(); (x) = 0;}
#endif

	template<class T> inline void swap(T & l, T & r)
	{
		T temp = l; l = r; r = temp;
	}

	template<class T> inline void limit(T & value
		, const T & minValue
		, const T & maxValue)
	{
		value = MAX((minValue), MIN((value), (maxValue)));
	}

	template<class T> inline T clamp(T x, T a, T b)
	{
		return MAX(a, MIN(x, b));
	}

	template<class T> inline T clamp0255(T x)
	{
		return clamp<T>(x, 0, 255);
	}

#define VIDEO_WIDTH		640
#define VIDEO_HEIGHT	480

}

#endif