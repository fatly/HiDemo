#include "stdafx.h"
#include "DSoundDevice.h"

#pragma comment(lib, "dsound.lib")

namespace e
{
#define MAX(a, b) ((a)<(b)?(b):(a))
#define DSOUND_DEFAULT_DEVICE 0

	template<class T> void SafeRelease(T** pp)
	{
		if (*pp)
		{
			(*pp)->Release();
			*pp = NULL;
		}
	}

	inline void MakeFormat(int nIndex, WAVEFORMATEX* pwfx)
	{
		int iSampleRate = nIndex / 4;
		int iType = nIndex % 4;

		switch (iSampleRate)
		{
		case 0: pwfx->nSamplesPerSec = 48000; break;
		case 1: pwfx->nSamplesPerSec = 44100; break;
		case 2: pwfx->nSamplesPerSec = 22050; break;
		case 3: pwfx->nSamplesPerSec = 11025; break;
		case 4: pwfx->nSamplesPerSec = 8000; break;
		}

		switch (iType)
		{
		case 0: pwfx->wBitsPerSample = 8; pwfx->nChannels = 1; break;
		case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
		case 2: pwfx->wBitsPerSample = 8; pwfx->nChannels = 2; break;
		case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
		}

		pwfx->wFormatTag = WAVE_FORMAT_PCM;
		pwfx->nBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
	}

	BOOL CALLBACK DSCaptureEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
	{
		IEnumDevice* fn = (IEnumDevice*)lpContext;
		if (fn)
		{
			fn->OnAddDevice(lpGUID, lpszDesc, lpszDrvName);
		}
		return TRUE;
	}

	BOOL CALLBACK DSCaptureEnumProc2(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
	{
		std::vector<DEVICEITEM>* pItems = (std::vector<DEVICEITEM>*)lpContext;
		if (pItems)
		{
			DEVICEITEM item(lpGUID, lpszDesc);
			pItems->push_back(item);
		}
		return TRUE;
	}

	CDSoundDevice::CDSoundDevice(CCritSec* pStateLock)
	{
		::CoInitialize(NULL);
		m_pDeviceGUID = NULL;//default device
		m_pDSCapture = NULL;
		m_pDSBuffer = NULL;
		m_pDSNotify = NULL;
		m_dwNotifySize = 0;
		m_dwCaptureSampleSize = kMaxSampleSize;
		m_dwCaptureBufferSize = 0;
		m_dwNextCaptureOffset = 0;
		m_pStateLock = pStateLock;

		memset(m_aPosNotify, 0, sizeof(DSBPOSITIONNOTIFY)*(NOTIFYS + 1));

		memset(&m_wfex, 0, sizeof(m_wfex));
		m_wfex.wFormatTag = WAVE_FORMAT_PCM;
		m_wfex.nChannels = kDefaultChannels;
		m_wfex.wBitsPerSample = KDefaultBitsPerSample;
		m_wfex.nSamplesPerSec = kDefaultSamplePerSec;
		m_wfex.nBlockAlign = m_wfex.nChannels * m_wfex.wBitsPerSample / BITS_PER_BYTE;
		m_wfex.nAvgBytesPerSec = m_wfex.nBlockAlign * m_wfex.nSamplesPerSec;

		m_hEventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(m_hEventNotify);

		//enum devices
		HRESULT hr = DirectSoundCaptureEnumerate(DSCaptureEnumProc2, (LPVOID)&m_DeviceList);
		if (FAILED(hr))
		{
			assert(0);
		}
	}


	CDSoundDevice::~CDSoundDevice(void)
	{
		if (m_hEventNotify)
		{
			CloseHandle(m_hEventNotify);
			m_hEventNotify = NULL;
		}

		::CoUninitialize();
	}

	HRESULT CDSoundDevice::GetDevices(IEnumDevice* pCallback)
	{
		CheckPointer(pCallback, E_POINTER);
		return DirectSoundCaptureEnumerate(DSCaptureEnumProc, (LPVOID)pCallback);
	}

	HRESULT CDSoundDevice::GetFormats(LPGUID pGUID, IEnumFormat* pCallback)
	{
		LPDIRECTSOUNDCAPTURE pDSCapture = NULL;
		LPDIRECTSOUNDCAPTUREBUFFER pDSBuffer = NULL;
		CheckPointer(pCallback, E_POINTER);

		HRESULT hr = ::DirectSoundCaptureCreate(pGUID, &pDSCapture, NULL);
		if (FAILED(hr)) return hr;

		WAVEFORMATEX wfex;
		memset(&wfex, 0, sizeof(wfex));
		DSCBUFFERDESC dscdes;
		memset(&dscdes, 0, sizeof(dscdes));
		dscdes.dwSize = sizeof(dscdes);

		for (int i = 0; i < 20; i++)
		{
			MakeFormat(i, &wfex);
			dscdes.dwBufferBytes = wfex.nAvgBytesPerSec;
			dscdes.lpwfxFormat = &wfex;
			hr = pDSCapture->CreateCaptureBuffer(&dscdes, &pDSBuffer, NULL);
			if (SUCCEEDED(hr))
			{
				if (pCallback) pCallback->OnAddFormat(wfex);
			}

			SafeRelease(&pDSBuffer);
		}

		SafeRelease(&pDSCapture);
		return NOERROR;
	}

	HRESULT CDSoundDevice::SetCaptureDevice(const LPGUID pGUID)
	{
		for (size_t i = 0; i < m_DeviceList.size(); i++)
		{
			if (!memcmp(m_DeviceList[i].pGUID, pGUID, sizeof(GUID)))
			{
				m_pDeviceGUID = m_DeviceList[i].pGUID;
				return S_OK;
			}
		}
		return E_INVALIDARG;
	}

	HRESULT CDSoundDevice::SetCaptureDevice(LPCTSTR pszDeviceName)
	{
		if (!pszDeviceName)
		{
			m_pDeviceGUID = NULL;
			return S_OK;
		}
		else
		{
			for (size_t i = 0; i < m_DeviceList.size(); i++)
			{
				if (!_tcscmp(pszDeviceName, m_DeviceList[i].szName))
				{
					m_pDeviceGUID = m_DeviceList[i].pGUID;
					return S_OK;
				}
			}
			return E_INVALIDARG;
		}
	}

	HRESULT CDSoundDevice::SetCaptureFormat(const WAVEFORMATEX& wfex)
	{
		if (m_wfex.nChannels == wfex.nChannels && 
			m_wfex.wBitsPerSample == wfex.wBitsPerSample && 
			m_wfex.nSamplesPerSec == wfex.nSamplesPerSec)
		{
			return S_FALSE;
		}

		WAVEFORMATEX wfx = wfex;
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		LPDIRECTSOUNDCAPTURE pCapture = NULL;
		LPDIRECTSOUNDCAPTUREBUFFER pBuffer = NULL;
		HRESULT hr = DirectSoundCaptureCreate(m_pDeviceGUID, &pCapture, NULL);

		if (SUCCEEDED(hr))
		{
			DSCBUFFERDESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.dwSize = sizeof(desc);
			desc.dwBufferBytes = wfx.nSamplesPerSec * (wfx.nChannels * wfx.wBitsPerSample / BITS_PER_BYTE);
			desc.lpwfxFormat = &wfx;
			hr = pCapture->CreateCaptureBuffer(&desc, &pBuffer, NULL);
		}

		if (SUCCEEDED(hr))
		{
			m_wfex = wfx;
			m_wfex.nBlockAlign = m_wfex.nChannels * m_wfex.wBitsPerSample / 8;
			m_wfex.nAvgBytesPerSec = m_wfex.nSamplesPerSec * m_wfex.nBlockAlign;
			m_wfex.cbSize = 0;
		}

		SafeRelease(&pBuffer);
		SafeRelease(&pCapture);
		return hr;
	}

	HRESULT CDSoundDevice::SetCaptureSampleSize(int nSampleSize /* = kMaxSampleSize */)
	{
		m_dwCaptureSampleSize = nSampleSize;
		return NOERROR;
	}

	HRESULT CDSoundDevice::CreateCaptureBuffer(WAVEFORMATEX& wfex)
	{
		HRESULT hr;
		DSCBUFFERDESC desc = { 0 };

		m_dwNotifySize = m_dwCaptureSampleSize;
		m_dwNotifySize -= m_dwNotifySize % m_wfex.nBlockAlign;
		m_dwCaptureBufferSize = m_dwNotifySize * NOTIFYS;

		ZeroMemory(&desc, sizeof(desc));
		desc.dwSize = sizeof(desc);
		desc.dwBufferBytes = m_dwCaptureBufferSize;
		desc.lpwfxFormat = &wfex;

		hr = m_pDSCapture->CreateCaptureBuffer(&desc, &m_pDSBuffer, NULL);
		if (FAILED(hr)) return hr;

		m_dwNextCaptureOffset = 0;
		hr = m_pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify);
		if (FAILED(hr)) return hr;

		for (int i = 0; i < NOTIFYS; i++)
		{
			m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
			m_aPosNotify[i].hEventNotify = m_hEventNotify;
		}

		hr = m_pDSNotify->SetNotificationPositions(NOTIFYS, m_aPosNotify);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	HRESULT CDSoundDevice::OnThreadCreate(void)
	{
		HRESULT hr = DirectSoundCaptureCreate(m_pDeviceGUID, &m_pDSCapture, NULL);
		if (FAILED(hr)) hr;

		return CreateCaptureBuffer(m_wfex);
	}

	HRESULT CDSoundDevice::OnThreadStartPlay(void)
	{
		CheckPointer(m_pDSBuffer, E_POINTER);
		m_dwNextCaptureOffset = 0;
		return m_pDSBuffer->Start(DSCBSTART_LOOPING);
	}

	HRESULT CDSoundDevice::OnThreadDestory(void)
	{
		SafeRelease(&m_pDSNotify);
		SafeRelease(&m_pDSBuffer);
		SafeRelease(&m_pDSCapture);
		m_dwNextCaptureOffset = 0;
		return NOERROR;
	}

	HRESULT CDSoundDevice::GetPCMFormat(WAVEFORMATEX* pwfex)
	{
		CheckPointer(pwfex, E_POINTER);
		memcpy(pwfex, &m_wfex, sizeof(WAVEFORMATEX));
		return S_OK;
	}

	HRESULT CDSoundDevice::GetPCMBuffer(void* pBuffer, int nMaxSize, int* nRead)
	{
		HRESULT hr;
		VOID*   pbCaptureData = NULL;
		DWORD   dwCaptureLength = 0;
		VOID*   pbCaptureData2 = NULL;
		DWORD   dwCaptureLength2 = 0;
		DWORD   dwReadPos = 0;
		DWORD   dwCapturePos = 0;
		DWORD   dwOffset = 0;
		LONG lLockSize = 0;

		CheckPointer(m_pDSBuffer, E_POINTER);
		//wait for notify event
		DWORD dwRet = 0;
		do{
			dwRet = ::WaitForSingleObject(m_hEventNotify, INFINITE);
			if (dwRet != WAIT_OBJECT_0)
			{
				return E_FAIL;
			}
		} while (dwRet != WAIT_OBJECT_0);

		if (FAILED(hr = m_pDSBuffer->GetCurrentPosition(&dwCapturePos, &dwReadPos)))
		{
			return hr;
		}

		lLockSize = dwReadPos - m_dwNextCaptureOffset;
		if (lLockSize < 0)
		{
			lLockSize += m_dwCaptureBufferSize;
		}

		// Block align lock size so that we are always write on a boundary
		lLockSize -= (lLockSize % m_dwNotifySize);

		if (lLockSize == 0) return NOERROR;
		if (lLockSize > nMaxSize) lLockSize = nMaxSize;

		// Lock the capture buffer down
		if (FAILED(hr = m_pDSBuffer->Lock(m_dwNextCaptureOffset
			, lLockSize
			, &pbCaptureData
			, &dwCaptureLength
			, &pbCaptureData2
			, &dwCaptureLength2
			, 0L)))
		{
			return hr;
		}

		if (pBuffer != NULL)
		{
			assert(dwCaptureLength <= (DWORD)nMaxSize);
			memcpy(pBuffer, pbCaptureData, dwCaptureLength);
			dwOffset += dwCaptureLength;
		}

		// Move the capture offset along
		m_dwNextCaptureOffset += dwCaptureLength;
		m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

		if (pbCaptureData2 != NULL)
		{		
			if (pBuffer != NULL)
			{
				assert(dwOffset + dwCaptureLength2 <= (DWORD)nMaxSize);
				memcpy((BYTE*)pBuffer + dwOffset, pbCaptureData2, dwCaptureLength2);
				dwOffset += dwCaptureLength2;
			}
			// Move the capture offset along
			m_dwNextCaptureOffset += dwCaptureLength2;
			m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
		}

		// Unlock the capture buffer
		hr = m_pDSBuffer->Unlock(pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);
		if (FAILED(hr)) return hr;

		*nRead = dwOffset;
		::ResetEvent(m_hEventNotify);
		return NOERROR;
	}
}
