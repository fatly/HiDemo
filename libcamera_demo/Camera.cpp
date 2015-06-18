#include "Camera.h"
#include <stdio.h>
#include <assert.h>
#include "FilterManager.h"

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	const TCHAR* deviceName = _T("@device:pnp:\\\\?\\usb#vid_045e&pid_0772&mi_00#7&1f8cc2a&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global");
	//const TCHAR* deviceName = _T("@device:sw:{860BB310-5D01-11D0-BD3B-00A0C911CE86}\\{E18AF7A1-06F5-45B5-832C-462F87B13A71}");
	
	Camera::Camera(void)
	{
		filterManager = new FilterManager();
		assert(filterManager);

		filterManager->SetDataType(_T("YUY2"));
		filterManager->SetDeviceName(deviceName);
		filterManager->SetFrameRate(15);
		filterManager->SetVideoSize(VIDEO_WIDTH, VIDEO_HEIGHT);
		filterManager->SetHandle(&Camera::_SampleHandle, this, HANDLE_TYPE_SAMPLE);
		filterManager->SetHandle(&Camera::_FormatHandle, this, HANDLE_TYPE_FORMAT);
	}

	Camera::~Camera(void)
	{
		filterManager->Stop();
		SAFE_DELETE(filterManager);
	}

	bool Camera::Start(void)
	{
		if (filterManager->IsStarted())
		{
			return true;
		}

		return SUCCEEDED(filterManager->Start());
	}

	void Camera::Stop(void)
	{
		filterManager->Stop();
	}

	bool Camera::Restart(void)
	{
		if (filterManager->IsStarted())
		{
			filterManager->Stop();
		}

		return SUCCEEDED(filterManager->Start());
	}

	void Camera::GetFormat(void)
	{
		filterManager->GetFormat();
	}

	void Camera::FormatHandle(TCHAR* type, int width, int height)
	{
		TCHAR buffer[256] = { 0 };
		_stprintf_s(buffer, _T("camera : type=%s,w=%d,h=%d\n"), type, width, height);
		OutputDebugString(buffer);
	}

	void Camera::SampleHandle(uchar* buffer, int width, int height, int bitCount)
	{
		static int count = 0; 
		TCHAR buf[256] = { 0 };
		_stprintf_s(buf, _T("camera : i=%d,w=%d,h=%d,bc=%d\n"), count++, width, height, bitCount);
		OutputDebugString(buf);
	}

	void Camera::_FormatHandle(void* param, TCHAR* type, int width, int height)
	{
		Camera* p = (Camera*)param;
		p->FormatHandle(type, width, height);
	}

	void Camera::_SampleHandle(void* param, uchar* buffer, int width, int height, int bitCount)
	{
		Camera* p = (Camera*)param;
		p->SampleHandle(buffer, width, height, bitCount);
	}
}

