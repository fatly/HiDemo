#ifndef __CORE_DEFINE_H__
#define __CORE_DEFINE_H__

// 不定长整形
typedef short int16;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned char byte;
// 定长整型
#if defined(_WIN64) || defined(__x86_64__)
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef long long intx;
	typedef unsigned long long uintx;
#else
	typedef int int32;
	typedef unsigned int uint32;
	typedef long long int64;
	typedef unsigned long long uint64;
	typedef int intx;
	typedef unsigned int uintx;
#endif

// #define WINDOW_SIZE			8192
// #define OVERLAP_COUNT		4
// #define POST_WINDOW_POWER	2
// 
// #define HALF_WINDOW			(WINDOW_SIZE / 2)
// #define OVERLAP_SIZE		(WINDOW_SIZE / OVERLAP_COUNT) //2048
// 
// #define TWOPI				6.283185307179586476925286766559
// #define INVSQRT2			0.70710678118654752440084436210485
// #define NODIVBYZERO			0.000000000000001

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a, b) ((a)<(b)?(b):(a))
#endif

#ifndef VIDEO_WIDTH 
#define VIDEO_WIDTH 640
#endif

#ifndef VIDEO_HEIGHT
#define VIDEO_HEIGHT 480
#endif

#ifndef WIDTHBYTES
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)
#endif

#endif