#ifndef __CORE_CAMERA_H__
#define __CORE_CAMERA_H__

#include "HandleDefs.h"

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
	private:
		virtual void FormatHandle(TCHAR* type, int width, int height);
		virtual void SampleHandle(uchar* buffer, int width, int height, int bitCount);

		static void _FormatHandle(void* param, TCHAR* type, int width, int height);
		static void _SampleHandle(void* param, uchar* buffer, int width, int height, int bitCount);
	private:
		FilterManager* filterManager;
	};
}

#endif
