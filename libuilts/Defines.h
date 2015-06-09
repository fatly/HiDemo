#ifndef __E_DEFINES_H__
#define __E_DEFINES_H__

namespace
{
	// 不定长整形
	typedef char int8;
	typedef short int16;
	typedef unsigned int uint;
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned short ushort;
	typedef unsigned char uchar;
	typedef unsigned char byte;

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
	typedef float real;

	// 其他定义
#ifdef __GNUC__
#	define override
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define ABS(a) (((a) < 0 ) ? -(a) : (a))

#define SAFE_RELEASE(x) {if(x){(x)->Release(); (x)=0;}}
#define SAFE_DELETE(x)  {if(x){delete (x); (x)=0;}}
}

#endif