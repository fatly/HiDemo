#ifndef __CORE_FILTERMANAGER_H__
#define __CORE_FILTERMANAGER_H__

#include <streams.h>
#include "libcamdefs.h"

namespace e
{
	class FilterManager
	{
	public:
		FilterManager(void);
		virtual ~FilterManager(void);
	public:
		HRESULT Start(void);
		HRESULT Stop(void);
		bool IsStarted(void) const;
		void SetVideoSize(int width, int height);
		void SetDataType(const TCHAR* type);
		void SetDeviceName(const TCHAR* name);
		void SetFrameRate(const int frameRate);
		void SetHandle(void* fnHandle, void* param, int type);
		void GetFormat(void);
		void GetDevice(void);
	private:
		HRESULT InitFilter(void);
		HRESULT CleanFilter(void);
		HRESULT RemoveFilter(IGraphBuilder* builder, IBaseFilter* filter);
		HRESULT SetFormat(void);
		HRESULT SetFrameRate(void);
		HRESULT EnumFormat(void);
		HRESULT EnumDevice(void);
	private:
		IGraphBuilder* graphBuilder;
		ICaptureGraphBuilder2* captureGraphBuilder2;
		IBaseFilter* videoCapture;
		IBaseFilter* videoRender;
		IBaseFilter* videoDecoder;
		TCHAR* dataType;
		TCHAR* deviceName;
		int videoWidth;
		int videoHeight;
		int frameRate;
		bool isStarted;

		SampleHanlde fnSampleHandle;
		void* fnSampleParam;

		FormatHandle fnFormatHandle;
		void* fnFormatParam;

		DeviceHandle fnDeviceHandle;
		void* fnDeviceParam;
	};
}

#endif
