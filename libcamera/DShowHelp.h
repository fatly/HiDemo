#ifndef __CORE_DSHOWHELP_H__
#define __CORE_DSHOWHELP_H__

#include <streams.h>

namespace e
{
	HRESULT GetMonikerByCLSID(_In_ REFCLSID rclsid, TCHAR* deviceName, IMoniker** pMoniker);

	HRESULT GetAMVideoConfig(IUnknown* pFilter, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig);

	HRESULT GetPinByDirection(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin);

	HRESULT GetMonikerByName(REFCLSID deviceCatogory, TCHAR* deviceName, IBaseFilter** pFilter);

	HRESULT GetCameraPin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin);

	HRESULT GetCameraConfig(IUnknown* pFilter, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig);

	HRESULT GetCrossbarPin(IUnknown* pUnk, PIN_DIRECTION wantDir, LPTSTR name, IPin** ppPin);

	HRESULT GetMonikerByDevName(REFCLSID deviceCatogory, TCHAR* deviceName, IMoniker** pMoniker);

	HRESULT GetMediaPin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin, bool bVideo);

	HRESULT GetMonikerByDisplayName(REFCLSID deviceCatogory, TCHAR* displayName, IBaseFilter** pFilter);

	HRESULT GetFirstDev(REFCLSID deviceCatogory, IBaseFilter** pFilter, bool bFirst);
}

#endif

