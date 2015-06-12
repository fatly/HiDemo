#include "FilterManager.h"
#include "RenderFilter.h"
#include "DShowHelp.h"

namespace e
{
	inline void GetMediaType(TCHAR* type, const GUID & uid)
	{
		if (::IsEqualCLSID(uid, MEDIASUBTYPE_IYUV))
		{
			_tcscpy(type, _T("IYUV"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_RGB24))
		{
			_tcscpy(type, _T("RGB24"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_RGB32))
		{
			_tcscpy(type, _T("RGB32"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_YV12))
		{
			_tcscpy(type, _T("YV12"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_IMC1))
		{
			_tcscpy(type, _T("IMC1"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_IMC2))
		{
			_tcscpy(type, _T("IMC2"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_IMC3))
		{
			_tcscpy(type, _T("IMC3"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_IMC4))
		{
			_tcscpy(type, _T("IMC4"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_UYVY))
		{
			_tcscpy(type, _T("UYVY"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_YUY2))
		{
			_tcscpy(type, _T("YUY2"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_YVYU))
		{
			_tcscpy(type, _T("YVYU"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_I420))
		{
			_tcscpy(type, _T("I420"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_HDYC))
		{
			_tcscpy(type, _T("HDYC"));
		}
		else if (::IsEqualCLSID(uid, MEDIASUBTYPE_MJPG))
		{
			_tcscpy(type, _T("MJPG"));
		}
		else
		{
			LPOLESTR lpszGUID = NULL;
			HRESULT hr = ::StringFromCLSID(uid, &lpszGUID);
			if (hr == S_OK && lpszGUID)
			{
				CoTaskMemFree(lpszGUID);
			}

			TCHAR buf[32] = { 0 };
			buf[0] = (uid.Data1 & 0x000000ff);
			buf[1] = (uid.Data1 & 0x0000ff00) >> 8;
			buf[2] = (uid.Data1 & 0x00ff0000) >> 16;
			buf[3] = (uid.Data1 & 0xff000000) >> 24;

			_tcscpy(type, buf);
		}
	}

	FilterManager::FilterManager(void)
	{
		graphBuilder = 0;
		captureGraphBuilder2 = 0;
		videoCapture = 0;
		videoDecoder = 0;
		videoRender = 0;
		videoWidth = 640;
		videoHeight = 480;
		frameRate = 25;
		isStarted = false;

		dataType = new TCHAR[MAX_PATH];
		memset(dataType, 0, sizeof(TCHAR) * MAX_PATH);

		deviceName = new TCHAR[MAX_PATH];
		memset(deviceName, 0, sizeof(TCHAR) * MAX_PATH);

		fnSampleHandle = 0;
		fnSampleParam = 0;

		fnFormatHandle = 0;
		fnFormatParam = 0;
	}

	FilterManager::~FilterManager(void)
	{
		CleanFilter();
	}

	HRESULT FilterManager::Start(void)
	{
		HRESULT hr = InitFilter();
		if (FAILED(hr))
		{
			CleanFilter();
		}

		return hr;
	}

	HRESULT FilterManager::Stop(void)
	{
		return CleanFilter();
	}

	bool FilterManager::IsStarted(void) const
	{
		return isStarted;
	}

	HRESULT FilterManager::InitFilter(void)
	{
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**)&graphBuilder);
		if (FAILED(hr)) return hr;

		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void**)&captureGraphBuilder2);
		if (FAILED(hr)) return hr;

		hr = captureGraphBuilder2->SetFiltergraph(graphBuilder);
		if (FAILED(hr)) return hr;

		hr = GetMonikerByDisplayName(CLSID_VideoInputDeviceCategory, deviceName, &videoCapture);
		if (FAILED(hr)) return hr;

		hr = graphBuilder->AddFilter(videoCapture, _T("Video Capture Filter"));
		if (FAILED(hr)) return hr;

		videoCapture->AddRef();

		hr = SetFrameRate();
		if (FAILED(hr)) return hr;

		hr = SetFormat();
		if (FAILED(hr)) return hr;

		videoRender = new RenderFilter(_T("Video Render Filter"), NULL, &hr);
		if (videoRender == 0)
		{
			return E_FAIL;
		}
		if (FAILED(hr)) hr;

		videoRender->AddRef();

		((RenderFilter*)videoRender)->SetSampleHandle(fnSampleHandle, fnSampleParam);

		hr = graphBuilder->AddFilter(videoRender, _T("Video Render Filter"));
		if (FAILED(hr)) return hr;

		if (_tcscmp(dataType, _T("MJPG")) == 0)
		{
			hr = CoCreateInstance(CLSID_MjpegDec, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&videoDecoder);
			if (FAILED(hr)) return hr;

			hr = graphBuilder->AddFilter(videoDecoder, TEXT("decompression"));
			if (FAILED(hr)) return hr;

			hr = captureGraphBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE
				, &MEDIATYPE_Video
				, videoCapture
				, videoDecoder
				, videoRender);
			if (FAILED(hr)) return hr;
		}
		else
		{
			hr = captureGraphBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE
				, &MEDIATYPE_Video
				, videoCapture
				, NULL
				, videoRender);
			if (FAILED(hr)) return hr;
		}

		//run
		IMediaControl* control = 0;
		hr = graphBuilder->QueryInterface(IID_IMediaControl, (void**)&control);
		if (FAILED(hr)) return hr;

		hr = control->Run();
		if (hr != S_OK)
		{
			if (hr == S_FALSE)
			{
				OAFilterState state;
				hr = control->GetState(INFINITE, &state);
				if (hr == S_OK)
				{
					isStarted = true;
				}
			}
		}
		else
		{
			isStarted = true;
		}

		SAFE_RELEASE(control);

		return hr;
	}

	HRESULT FilterManager::CleanFilter(void)
	{
		HRESULT hr = S_FALSE;
		if (graphBuilder == 0) return hr;

		IMediaControl* control;
		hr = graphBuilder->QueryInterface(IID_IMediaControl, (void**)&control);
		if (SUCCEEDED(hr))
		{
			control->Stop();
			SAFE_RELEASE(control);
		}

		if (videoRender)
		{
			RemoveFilter(graphBuilder, videoRender);
			hr = graphBuilder->RemoveFilter(videoRender);
		}

		if (videoDecoder)
		{
			RemoveFilter(graphBuilder, videoDecoder);
			hr = graphBuilder->RemoveFilter(videoDecoder);
		}

		if (videoCapture)
		{
			RemoveFilter(graphBuilder, videoCapture);
			hr = graphBuilder->RemoveFilter(videoCapture);
		}

		SAFE_RELEASE(videoRender);
		SAFE_RELEASE(videoDecoder);
		SAFE_RELEASE(videoCapture);
		SAFE_RELEASE(captureGraphBuilder2);

		isStarted = false;

		return hr;
	}

	HRESULT FilterManager::RemoveFilter(IGraphBuilder* filterGraph, IBaseFilter* filter)
	{
		if (filter == 0) return E_FAIL;

		IPin *pFrom = 0;
		IPin *pTo = 0;
		ULONG u;
		IEnumPins *pins = NULL;
		PIN_INFO pininfo;

		HRESULT hr = filter->EnumPins(&pins);
		if (FAILED(hr)) return hr;

		pins->Reset();

		while (hr == NOERROR)
		{
			hr = pins->Next(1, &pFrom, &u);

			if (hr == S_OK && pFrom)
			{
				pFrom->ConnectedTo(&pTo);

				if (pTo)
				{
					hr = pTo->QueryPinInfo(&pininfo);

					if (hr == NOERROR)
					{
						if (pininfo.dir == PINDIR_INPUT)
						{
							RemoveFilter(filterGraph, pininfo.pFilter);
							filterGraph->Disconnect(pTo);
							filterGraph->Disconnect(pFrom);
							filterGraph->RemoveFilter(pininfo.pFilter);
						}
						pininfo.pFilter->Release();
					}
					pTo->Release();
				}
				pFrom->Release();
			}
		}
		
		SAFE_RELEASE(pins);

		return S_OK;
	}

	HRESULT FilterManager::SetFormat(void)
	{
		if (videoCapture == 0) return E_FAIL;

		IAMStreamConfig* config;
		HRESULT hr = GetAMVideoConfig(videoCapture, PINDIR_OUTPUT, &config);
		if (FAILED(hr)) return hr;

		AM_MEDIA_TYPE* mt;
		hr = config->GetFormat(&mt);
		if (FAILED(hr))
		{
			config->Release();
			return hr;
		}

		TCHAR* type = dataType;

		if (0 == _tcscmp(type, _T("IYUV")))
		{
			mt->subtype = MEDIASUBTYPE_IYUV;
		}
		else if (0 == _tcscmp(type, _T("RGB24")))
		{
			mt->subtype = MEDIASUBTYPE_RGB24;
		}
		else if (0 == _tcscmp(type, _T("RGB32")))
		{
			mt->subtype = MEDIASUBTYPE_RGB32;
		}
		else if (0 == _tcscmp(type, _T("YV12")))
		{
			mt->subtype = MEDIASUBTYPE_YV12;
		}
		else if (0 == _tcscmp(type, _T("IMC1")))
		{
			mt->subtype = MEDIASUBTYPE_IMC1;
		}
		else if (0 == _tcscmp(type, _T("IMC2")))
		{
			mt->subtype = MEDIASUBTYPE_IMC2;
		}
		else if (0 == _tcscmp(type, _T("IMC3")))
		{
			mt->subtype = MEDIASUBTYPE_IMC3;
		}
		else if (0 == _tcscmp(type, _T("IMC4")))
		{
			mt->subtype = MEDIASUBTYPE_IMC4;
		}
		else if (0 == _tcscmp(type, _T("UYVY")))
		{
			mt->subtype = MEDIASUBTYPE_UYVY;
		}
		else if (0 == _tcscmp(type, _T("YUY2")))
		{
			mt->subtype = MEDIASUBTYPE_YUY2;
		}
		else if (0 == _tcscmp(type, _T("YVYU")))
		{
			mt->subtype = MEDIASUBTYPE_YVYU;
		}
		else if (0 == _tcscmp(type, _T("I420")))
		{
			mt->subtype = MEDIASUBTYPE_I420;
		}
		else if (0 == _tcscmp(type, _T("MJPG")))
		{
			mt->subtype = MEDIASUBTYPE_MJPG;
		}

		((VIDEOINFOHEADER*)(mt->pbFormat))->bmiHeader.biWidth = videoWidth;
		((VIDEOINFOHEADER*)(mt->pbFormat))->bmiHeader.biHeight = videoHeight;

		hr = config->SetFormat(mt);

		config->Release();

		return hr;		
	}

	HRESULT FilterManager::SetFrameRate(void)
	{
		if (!videoCapture) return E_FAIL;

		IAMStreamConfig* config = 0;
		HRESULT hr = GetAMVideoConfig(videoCapture, PINDIR_OUTPUT, &config);

		if (SUCCEEDED(hr))
		{
			AM_MEDIA_TYPE* mt;
			config->GetFormat(&mt);

			if (SUCCEEDED(hr))
			{
				((VIDEOINFOHEADER*)(mt->pbFormat))->AvgTimePerFrame = (LONGLONG)(10000000 / frameRate);
				hr = config->SetFormat(mt);
				if (SUCCEEDED(hr))
				{
					this->frameRate = frameRate;
				}
			}

			config->Release();
		}

		return hr;
	}

	HRESULT FilterManager::EnumFormat(void)
	{
		if (videoCapture == 0) return E_FAIL;

		IAMStreamConfig* config = 0;
		HRESULT hr = GetAMVideoConfig(videoCapture, PINDIR_OUTPUT, &config);
		if (FAILED(hr)) return hr;

		int count = 0, size = 0;
		hr = config->GetNumberOfCapabilities(&count, &size);
		if (FAILED(hr))
		{
			config->Release();
			return hr;
		}

		TCHAR type[MAX_PATH] = { 0 };
		AM_MEDIA_TYPE* mt = 0;
		VIDEO_STREAM_CONFIG_CAPS caps;
		for (int i = 0; i < count; i++)
		{
			hr = config->GetStreamCaps(i, &mt, (BYTE*)&caps);
			if (SUCCEEDED(hr))
			{
				memset(type, 0, sizeof(TCHAR)*MAX_PATH);
				int width = ((VIDEOINFOHEADER*)(mt)->pbFormat)->bmiHeader.biWidth;
				int height = ((VIDEOINFOHEADER*)(mt)->pbFormat)->bmiHeader.biHeight;

				if (width <= 0 || height <= 0) continue;

				GetMediaType(type, mt->subtype);
				//回调给调用者
				if (fnFormatHandle)
				{
					fnFormatHandle(fnSampleParam, type, width, height);
				}
			}
		}

		SAFE_RELEASE(config);

		return S_OK;
	}

	void FilterManager::SetVideoSize(int width, int height)
	{
		this->videoWidth = width;
		this->videoHeight = height;
	}

	void FilterManager::SetDataType(const TCHAR* type)
	{
		_tcscpy(this->dataType, type);
	}

	void FilterManager::SetDeviceName(const TCHAR* name)
	{
		_tcscpy(this->deviceName, name);
	}

	void FilterManager::SetFrameRate(const int frameRate)
	{
		this->frameRate = frameRate;
	}

	void FilterManager::GetFormat(void)
	{
		EnumFormat();
	}

	void FilterManager::SetHandle(void* fnHandle, void* param, int type)
	{
		if (type == HANDLE_TYPE_FORMAT)
		{
			fnFormatHandle = (FormatHandle)fnHandle;
			fnSampleParam = param;
		}
		else if (type == HANDLE_TYPE_SAMPLE)
		{
			fnSampleHandle = (SampleHanlde)fnHandle;
			fnSampleParam = param;
		}
	}

}


