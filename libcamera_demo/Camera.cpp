#include "Camera.h"
#include <stdio.h>
#include <assert.h>
#include "FilterManager.h"
#include "libutils.h"

#pragma comment(lib, "libutils.lib")

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	const TCHAR* deviceName = _T("@device:pnp:\\\\?\\usb#vid_045e&pid_0772&mi_00#6&368b747d&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global");
	
	Camera::Camera()
	{
		filterManager = new FilterManager();
		assert(filterManager);

		filterManager->SetDataType(_T("YUY2"));
		filterManager->SetDeviceName(deviceName);
		filterManager->SetVideoSize(VIDEO_WIDTH, VIDEO_HEIGHT);
		filterManager->SetHandle(&Camera::SampleHandle, this, HANDLE_TYPE_SAMPLE);
		filterManager->SetHandle(&Camera::FormatHandle, this, HANDLE_TYPE_FORMAT);
	}

	Camera::~Camera()
	{
		filterManager->Stop();
		SAFE_DELETE(filterManager);
	}

	bool Camera::Start(void)
	{
		return SUCCEEDED(filterManager->Start());
	}

	void Camera::Stop(void)
	{
		filterManager->Stop();
	}

	void Camera::GetFormat(void)
	{
		filterManager->GetFormat();
	}

	void Camera::_FormatHandle(TCHAR* type, int width, int height)
	{
		TCHAR buffer[256] = { 0 };
		_stprintf_s(buffer, _T("type=%s,w=%d,h=%d\n"), type, width, height);
		OutputDebugString(buffer);
	}

	void Camera::_SampleHandle(uchar* buffer, int width, int height, int bitCount)
	{
		static int count = 0; 
		TCHAR buf[256] = { 0 };
		_stprintf_s(buf, _T("i=%d,w=%d,h=%d,bc=%d\n"), count++, width, height, bitCount);
		OutputDebugString(buf);
		
		if (count % 25 == 0)
		{
			char fileName[MAX_PATH] = { 0 };
			sprintf_s(fileName, "f:\\video\\%d.bmp", 1);
			e::Bitmap bitmap(width, height, bitCount, buffer);
			bitmap.SwapChannel(0, 2);
			bitmap.Save(fileName);
		}
	}

	void Camera::FormatHandle(void* param, TCHAR* type, int width, int height)
	{
		Camera* p = (Camera*)param;
		p->_FormatHandle(type, width, height);
	}

	void Camera::SampleHandle(void* param, uchar* buffer, int width, int height, int bitCount)
	{
		Camera* p = (Camera*)param;
		p->_SampleHandle(buffer, width, height, bitCount);
	}
}

