#ifndef __CORE_CAMERA_H__
#define __CORE_CAMERA_H__

#include "libcamdefs.h"

namespace e
{
	class FilterManager;

	class Camera
	{
	public:
		Camera(void);
		virtual ~Camera(void);
	public:
		bool Start(void);
		void Stop(void);
		bool Restart(void);
		void GetFormat(void);
		virtual void OnVideoFormat(TCHAR* type, int width, int height);
		virtual void OnVideoSample(uchar* buffer, int size, int width, int height, int bitCount);
	protected:
		static void FormatProxy(void* param, TCHAR* type, int width, int height);
		static void SampleProxy(void* param, uchar* buffer, int size, int width, int height, int bitCount);
	protected:
		FilterManager* filterManager;
	};
}

#endif
