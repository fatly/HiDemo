#pragma once
#include <InitGuid.h>
//#include "DynamicSource.h"
#include "DSoundDevice.h"

namespace e
{
	//{8b252d69-81f1-4eb4-b9ac-be73b2080a7b}
	DEFINE_GUID(CLSID_DSoundCaptureFilter,
		0x8b252d69, 0x81f1, 0x4eb4, 0xb9, 0xac, 0xbe, 0x73, 0xb2, 0x08, 0x0a, 0x7b);

	class CDSoundCapture : public CSource
	{
	public:
		CDSoundCapture(LPCTSTR lpszDeviceName, LPUNKNOWN lpUnk, HRESULT* phr);
		virtual ~CDSoundCapture(void);
		HRESULT SetCaptureFormat(const WAVEFORMATEX& wfex);
		HRESULT SetCaptureDevice(LPCTSTR pszDeviceName);
		HRESULT SetCaptureSampleSize(const int nBufferSize);
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	public:
		CDSoundDevice* m_pDevice;
	};

	class CDSoundStream
		: public CSourceStream
		, public IAMStreamConfig
		, public IKsPropertySet
	{
	public:
		CDSoundStream(LPCTSTR lpszDeviceName, HRESULT* phr, CDSoundCapture* pParent, LPCWCHAR pPinName);
		virtual ~CDSoundStream(void);
		//IUnknown
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
		//IQualityControl
		STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
		//IAMStreamConfig
		STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
		STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppmt);
		STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
		STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppmt, BYTE *pSCC);
		//IKsPropertySet
		STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
		STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
		STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
	public:
		// stuff an audio buffer with the current format
		HRESULT FillBuffer(IMediaSample *pms);
		// ask for buffers of the size appropriate to the agreed media type.
		HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
		HRESULT CheckMediaType(const CMediaType *pMediaType);
		HRESULT GetMediaType(int iPosition, CMediaType *pmt);
		HRESULT SetMediaType(const CMediaType *pmt);
		//virtual function
		HRESULT OnThreadCreate(void);
		HRESULT OnThreadDestroy(void);
		HRESULT OnThreadStartPlay(void);
	protected:
		HRESULT GetMediaType(CMediaType *pmt);
		void SetupWaveFormat(int iIndex, WAVEFORMATEX* pwfx);
	protected:
		//CCritSec			m_cSharedState;
		CRefTime			m_rtSampleTime;    // The time to be stamped on each sample
		LONGLONG		m_llSampleMediaTimeStart;
		CDSoundCapture* m_pParent;
		CDSoundDevice* m_pDevice;
	};
}
