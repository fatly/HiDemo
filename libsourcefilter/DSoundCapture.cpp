#include "stdafx.h"
#include "DSoundCapture.h"
#include "Constants.h"

namespace e
{
	CDSoundCapture::CDSoundCapture(LPCTSTR lpszDeviceName, LPUNKNOWN lpUnk, HRESULT* phr)
		:CSource(NAME("DSoundCapture Filter"), lpUnk, CLSID_DSoundCaptureFilter, phr)
		, m_pDevice(NULL)
	{
		CAutoLock l(&m_cStateLock);
		m_paStreams = (CSourceStream **) new CDSoundStream*[1];
		if (m_paStreams == NULL) {
			if (phr)
				*phr = E_OUTOFMEMORY;
			return;
		}

		m_paStreams[0] = new CDSoundStream(lpszDeviceName, phr, this, L"DSound Stream");
		if (m_paStreams[0] == NULL) 
		{
			if (phr) *phr = E_OUTOFMEMORY;
			return;
		}
		m_iPins = 1;		
	}

	CDSoundCapture::~CDSoundCapture(void)
	{

	}

	HRESULT CDSoundCapture::SetCaptureFormat(const WAVEFORMATEX& wfex)
	{
		CAutoLock l(&m_cStateLock);
		CheckPointer(m_pDevice, E_POINTER);
		return m_pDevice->SetCaptureFormat(wfex);
	}

	HRESULT CDSoundCapture::SetCaptureDevice(LPCTSTR pszDeviceName)
	{
		CAutoLock l(&m_cStateLock);
		CheckPointer(m_pDevice, E_POINTER);
		return m_pDevice->SetCaptureDevice(pszDeviceName);
	}

	HRESULT CDSoundCapture::SetCaptureSampleSize(const int nSampleSize)
	{
		CheckPointer(m_pDevice, E_POINTER);
		return m_pDevice->SetCaptureSampleSize(nSampleSize);
	}

	STDMETHODIMP CDSoundCapture::QueryInterface(REFIID riid, __deref_out void **ppv)
	{
		if (riid == _uuidof(IAMStreamConfig) || riid == _uuidof(IKsPropertySet))
			return m_paStreams[0]->QueryInterface(riid, ppv);
		else
			return CSource::QueryInterface(riid, ppv);
	}

	//////////////////////////////////////////////////////////////////////////
	//--------------------------------------CDSoundSourceStream-------------------------------//
	//////////////////////////////////////////////////////////////////////////
	CDSoundStream::CDSoundStream(LPCTSTR lpszDeviceName, HRESULT* phr, CDSoundCapture* pParent, LPCWCHAR pPinName)
		: CSourceStream(NAME("DirectSound output pin"), phr, pParent, pPinName)
		, m_llSampleMediaTimeStart(0)
		, m_pParent(pParent)
	{
		ASSERT(phr);
		m_pDevice = new CDSoundDevice(pParent->pStateLock());
		if (m_pDevice == NULL)
		{
			if (phr) *phr = E_OUTOFMEMORY;
			return;
		}

		HRESULT hr = m_pDevice->SetCaptureDevice(lpszDeviceName);
		if (FAILED(hr))
		{
			if (phr) *phr = E_INVALIDARG;
			return;
		}
		pParent->m_pDevice = m_pDevice;
	}

	CDSoundStream::~CDSoundStream(void)
	{
		if (m_pDevice) delete m_pDevice;
	}

	STDMETHODIMP_(ULONG) CDSoundStream::AddRef(void)
	{
		return GetOwner()->AddRef();
	}

	STDMETHODIMP_(ULONG) CDSoundStream::Release(void)
	{
		return GetOwner()->Release();
	}

	STDMETHODIMP CDSoundStream::QueryInterface(REFIID riid, __deref_out void **ppv)
	{
		CheckPointer(ppv, E_POINTER);
		if (riid == _uuidof(IAMStreamConfig))
			*ppv = (IAMStreamConfig*)this;
		else if (riid == _uuidof(IKsPropertySet))
			*ppv = (IKsPropertySet*)this;
		else
			return CSourceStream::QueryInterface(riid, ppv);
		AddRef();
		return S_OK;
	}

	STDMETHODIMP CDSoundStream::Notify(IBaseFilter * pSender, Quality q)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP CDSoundStream::SetFormat(AM_MEDIA_TYPE *pmt)
	{
		CheckPointer(pmt, E_POINTER);
		return m_pDevice->SetCaptureFormat(*(WAVEFORMATEX*)(pmt->pbFormat));
	}

	STDMETHODIMP CDSoundStream::GetFormat(AM_MEDIA_TYPE** ppmt)
	{
		CheckPointer(ppmt, E_POINTER);
		WAVEFORMATEX wfex;
		HRESULT hr = m_pDevice->GetPCMFormat(&wfex);
		if (FAILED(hr)) return hr;

		*ppmt = CreateMediaType(&m_mt);
		if (*ppmt == NULL) return E_OUTOFMEMORY;

		return CreateAudioMediaType(&wfex, *ppmt, TRUE);
	}

	STDMETHODIMP CDSoundStream::GetNumberOfCapabilities(__out int *piCount, __out int *piSize)
	{
		CheckPointer(piCount&&piSize, E_POINTER);
		*piCount = 20;
		*piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);
		return S_OK;
	}

	STDMETHODIMP CDSoundStream::GetStreamCaps(int iIndex, __out AM_MEDIA_TYPE **ppmt, __out BYTE *pSCC)
	{
		CheckPointer(ppmt, E_POINTER);
		if (iIndex < 0) return E_INVALIDARG;
		if (iIndex >= 20) return VFW_S_NO_MORE_ITEMS;
		HRESULT hr = GetFormat(ppmt);
		if (FAILED(hr)) return hr;
		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)(*ppmt)->pbFormat;
		SetupWaveFormat(iIndex, pwfx);
		return S_OK;
	}

	STDMETHODIMP CDSoundStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP CDSoundStream::Get(REFGUID guidPropSet
		, DWORD dwPropID
		, void *pInstanceData
		, DWORD cbInstanceData
		, void *pPropData
		, DWORD cbPropData
		, DWORD *pcbReturned)
	{
		if (guidPropSet != AMPROPSETID_Pin)  return E_PROP_SET_UNSUPPORTED;
		if (dwPropID != AMPROPERTY_PIN_CATEGORY)  return E_PROP_ID_UNSUPPORTED;
		if (pPropData == NULL && pcbReturned == NULL)  return E_POINTER;

		if (pcbReturned) *pcbReturned = sizeof(GUID);
		if (pPropData == NULL)  return S_OK;
		if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;

		*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
		return S_OK;
	}

	STDMETHODIMP CDSoundStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
	{
		if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
		if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
		if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET;
		return S_OK;
	}

	void CDSoundStream::SetupWaveFormat(int iIndex, WAVEFORMATEX* pwfx)
	{
		int nRate = iIndex / 4;
		int nType = iIndex % 4;

		switch (nRate)
		{
		case 0: pwfx->nSamplesPerSec = 48000; break;
		case 1: pwfx->nSamplesPerSec = 44100; break;
		case 2: pwfx->nSamplesPerSec = 22050; break;
		case 3: pwfx->nSamplesPerSec = 11025; break;
		case 4: pwfx->nSamplesPerSec = 8000; break;
		}

		switch (nType)
		{
		case 0: pwfx->wBitsPerSample = 8; pwfx->nChannels = 1; break;
		case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
		case 2: pwfx->wBitsPerSample = 8; pwfx->nChannels = 2; break;
		case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
		}

		pwfx->nBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / BITS_PER_BYTE);
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
	}

	HRESULT CDSoundStream::CheckMediaType(const CMediaType *pMediaType)
	{
		CheckPointer(pMediaType, E_POINTER);
		CAutoLock lock(m_pFilter->pStateLock());

		CMediaType mt;
		GetMediaType(&mt);

		if (mt == *pMediaType)
		{
			return NOERROR;
		}

		return E_FAIL;
	}

	HRESULT CDSoundStream::GetMediaType(CMediaType *pmt)
	{
		CheckPointer(pmt, E_POINTER);
		ASSERT(CritCheckIn(m_pFilter->pStateLock()));

		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
		if (pwfx == NULL) return E_OUTOFMEMORY;
		//此处设置输出格式
		HRESULT hr = m_pDevice->GetPCMFormat(pwfx);
		if (FAILED(hr)) return hr;

		return CreateAudioMediaType(pwfx, pmt, FALSE);
	}

	HRESULT CDSoundStream::GetMediaType(int iPosition, CMediaType *pmt)
	{
		CheckPointer(pmt, E_POINTER);
		CAutoLock lock(m_pFilter->pStateLock());
		if (iPosition < 0) return E_INVALIDARG;
		if (iPosition >= 20) return VFW_S_NO_MORE_ITEMS;
		HRESULT hr = GetMediaType(pmt);
		if (FAILED(hr)) return hr;
		SetupWaveFormat(iPosition, (WAVEFORMATEX*)pmt->pbFormat);
		return NOERROR;
	}

	HRESULT CDSoundStream::SetMediaType(const CMediaType *pmt)
	{
		CheckPointer(pmt, E_POINTER);
		CAutoLock lock(m_pFilter->pStateLock());

		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)pmt->pbFormat;
		HRESULT hr = m_pDevice->SetCaptureFormat(*pwfx);
		if (FAILED(hr)) return hr;
		return CSourceStream::SetMediaType(pmt);
	}

	HRESULT CDSoundStream::FillBuffer(IMediaSample *pms)
	{
		CheckPointer(pms, E_POINTER);
		CAutoLock lock(m_pParent->pStateLock());

		BYTE* pData = NULL;
		HRESULT hr = pms->GetPointer(&pData);
		if (FAILED(hr)) return hr;

		WAVEFORMATEX* pwfexCurrent = (WAVEFORMATEX*)m_mt.Format();
		ASSERT(pwfexCurrent->wFormatTag == WAVE_FORMAT_PCM);
		if (pwfexCurrent->wFormatTag != WAVE_FORMAT_PCM) return E_FAIL;
		//此处给出采集数据
		int nActualSize = 0;
		hr = m_pDevice->GetPCMBuffer(pData, pms->GetSize(), &nActualSize);
		if (FAILED(hr)) return hr;
		hr = pms->SetActualDataLength(nActualSize);
		if (FAILED(hr)) return hr;
		WriteLog(_T("Debug"), _T("---------------------------------audio samples capture : %d--------------------------------------"), nActualSize);

		CRefTime rtStart = m_rtSampleTime;
		m_rtSampleTime = rtStart + (REFERENCE_TIME)(UNITS * pms->GetActualDataLength()) /
			(REFERENCE_TIME)pwfexCurrent->nAvgBytesPerSec;
		hr = pms->SetTime((REFERENCE_TIME*)&rtStart, (REFERENCE_TIME*)&m_rtSampleTime);
		if (FAILED(hr)) return hr;

// 		hr = pms->SetPreroll(FALSE);
// 		if (FAILED(hr)) return hr;
// 
// 		hr = pms->SetMediaType(NULL);
// 		if (FAILED(hr)) return hr;

		LONGLONG llMediaTimeStart = m_llSampleMediaTimeStart;
		DWORD dwNumAudioSamplesInPacket = (pms->GetActualDataLength() * BITS_PER_BYTE) /
			(pwfexCurrent->nChannels * pwfexCurrent->wBitsPerSample);
		LONGLONG llMediaTimeStop = m_llSampleMediaTimeStart + dwNumAudioSamplesInPacket;
		hr = pms->SetMediaTime(&llMediaTimeStart, &llMediaTimeStop);
		if (FAILED(hr)) return hr;
		m_llSampleMediaTimeStart = llMediaTimeStop;

		hr = pms->SetSyncPoint(TRUE);
		if (FAILED(hr)) return hr;

		return NOERROR;
	}

	HRESULT CDSoundStream::DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties)
	{	
		CheckPointer(pIMemAlloc, E_POINTER);
		CheckPointer(pProperties, E_POINTER);
		CAutoLock lock(m_pFilter->pStateLock());

		WAVEFORMATEX *pwfexCurrent = (WAVEFORMATEX *)m_mt.Format();
		ASSERT(pwfexCurrent->wFormatTag == WAVE_FORMAT_PCM);
		pProperties->cBuffers = 1;
		pProperties->cbBuffer = kMaxBufferSize;

		// Ask the allocator to reserve us the memory
		ALLOCATOR_PROPERTIES Actual;
		HRESULT hr = pIMemAlloc->SetProperties(pProperties, &Actual);
		if (FAILED(hr)) return hr;

		// Is this allocator unsuitable
		if (Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
		return NOERROR;
	}

	HRESULT CDSoundStream::OnThreadCreate(void)
	{
		CheckPointer(m_pDevice, E_POINTER);
		return m_pDevice->OnThreadCreate();
	}

	HRESULT CDSoundStream::OnThreadDestroy(void)
	{
		CheckPointer(m_pDevice, E_POINTER);
		return m_pDevice->OnThreadDestory();
	}

	HRESULT CDSoundStream::OnThreadStartPlay(void)
	{
		CheckPointer(m_pDevice, E_POINTER);
		//sample时间, 单位为100ns
		m_rtSampleTime = 0;// ((REFERENCE_TIME)GetTickCount()) * 10000;//100 ns
		m_llSampleMediaTimeStart = 0;
		return m_pDevice->OnThreadStartPlay();
	}
}