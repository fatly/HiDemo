#include "stdafx.h"
#include "GraphFilter.h"
#include "DSoundCapture.h"
#include "AudioRender.h"

namespace e
{
	template<class T>
	void SafeRelease(T** pp)
	{
		if (*pp)
		{
			(*pp)->Release();
			*pp = NULL;
		}
	}

	CGraphFilter::CGraphFilter()
	{
		m_pGraphBuilder = NULL;
		m_pCaptureGraphBuilder2 = NULL;
		m_pAudioSource = NULL;
		m_pAudioRender = NULL;
	}

	CGraphFilter::~CGraphFilter()
	{
		Stop();
		Destroy();
	}

	HRESULT CGraphFilter::Create(void)
	{
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**)&m_pGraphBuilder);
		if (FAILED(hr)) return hr;

		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void**)&m_pCaptureGraphBuilder2);
		if (FAILED(hr)) return hr;

		hr = m_pCaptureGraphBuilder2->SetFiltergraph(m_pGraphBuilder);
		if (FAILED(hr)) return hr;

		m_pAudioSource = new CDSoundCapture(NULL, NULL, &hr);
		if (m_pAudioSource == NULL) return E_OUTOFMEMORY;
		if (FAILED(hr)) return hr;

		hr = m_pGraphBuilder->AddFilter(m_pAudioSource, _T("AudioSource"));
		if (FAILED(hr)) return hr;
		m_pAudioSource->AddRef();

		m_pAudioRender = new CAudioRender(_T("AudioRender"), NULL, &hr);
		if (m_pAudioRender == NULL) return E_OUTOFMEMORY;
		if (FAILED(hr)) return hr;

		hr = m_pGraphBuilder->AddFilter(m_pAudioRender, _T("AudioRender"));
		if (FAILED(hr)) return hr;
		m_pAudioRender->AddRef();

		hr = m_pCaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE
			, NULL
			, m_pAudioSource
			, NULL
			, m_pAudioRender);
		if (FAILED(hr)) return hr;

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
		if (FAILED(hr)) return hr;

		return Pause();
	}

	HRESULT CGraphFilter::Start(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (FAILED(hr)) return hr;

		if (fs != State_Running)
		{
			hr = m_pMediaControl->Run();
		}

		return hr;
	}

	HRESULT CGraphFilter::Pause(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (FAILED(hr)) return hr;

		if (fs != State_Paused)
		{
			hr = m_pMediaControl->Pause();
		}

		return hr;
	}

	HRESULT CGraphFilter::Stop(void)
	{
		CheckPointer(m_pMediaControl, E_POINTER);
		OAFilterState fs;
		HRESULT hr = m_pMediaControl->GetState(0, &fs);
		if (FAILED(hr)) return hr;

		if (fs != State_Stopped)
		{
			hr = m_pMediaControl->Stop();
		}

		return hr;
	}

	HRESULT CGraphFilter::Destroy(void)
	{
		SafeRelease(&m_pMediaControl);
		SafeRelease(&m_pAudioSource);
		SafeRelease(&m_pAudioRender);
		SafeRelease(&m_pCaptureGraphBuilder2);
		SafeRelease(&m_pGraphBuilder);
		return NOERROR;
	}
}
