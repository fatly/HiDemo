#ifndef __CORE_CAMERA_H__
#define __CORE_CAMERA_H__

#include "HandleDefs.h"

namespace e
{
	class FilterManager;

	class Camera
	{
	public:
		Camera();
		~Camera();
	public:
		bool Start(void);
		void Stop(void);
		void GetFormat(void);
	private:
		void _FormatHandle(TCHAR* type, int width, int height);
		void _SampleHandle(uchar* buffer, int width, int height, int bitCount);

		static void FormatHandle(void* param, TCHAR* type, int width, int height);
		static void SampleHandle(void* param, uchar* buffer, int width, int height, int bitCount);
	private:
		FilterManager* filterManager;
	};
}

#endif
