#include "stdafx.h"
#include "device.h"

namespace e
{
#define MAX(a, b) ((a)<(b)?(b):(a))

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

	CCaptureDevice::CCaptureDevice(void)
	{
		::CoInitialize(NULL);
		m_pDSCapture = NULL;
		m_pDSBuffer = NULL;
		m_pDSNotify = NULL;
		m_hThread = NULL;
		m_bStarted = FALSE;
		m_dwNotifySize = 0;
		m_dwCaptureBufferSize = 0;
		m_dwNextCaptureOffset = 0;
		m_pSampleCallback = NULL;
		m_pGUID = NULL;//default device

		memset(&m_wfex, 0, sizeof(m_wfex));
		memset(m_aPosNotify, 0, sizeof(DSBPOSITIONNOTIFY)*(NOTIFYS + 1));

		m_wfex.wFormatTag = WAVE_FORMAT_PCM;
		m_wfex.nChannels = 2;
		m_wfex.wBitsPerSample = 16;
		m_wfex.nSamplesPerSec = 44100;
		m_wfex.nBlockAlign = m_wfex.nChannels * m_wfex.wBitsPerSample / 8;
		m_wfex.nAvgBytesPerSec = m_wfex.nBlockAlign * m_wfex.nSamplesPerSec;

		m_hEventStop = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(m_hEventStop);
		m_hEventNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(m_hEventNotify);
	}


	CCaptureDevice::~CCaptureDevice(void)
	{
		Close();

		if (m_hEventStop)
		{
			CloseHandle(m_hEventStop);
			m_hEventStop = NULL;
		}

		if (m_hEventNotify)
		{
			CloseHandle(m_hEventNotify);
			m_hEventNotify = NULL;
		}

		::CoUninitialize();
	}

	HRESULT CCaptureDevice::GetDevices(IEnumDevice* pCallback)
	{
		if (pCallback == NULL) return E_POINTER;
		HRESULT hr = DirectSoundCaptureEnumerate(DSCaptureEnumProc, (LPVOID)pCallback);
		if (FAILED(hr))
		{
			assert(0);
		}
		return hr;
	}

	HRESULT CCaptureDevice::GetFormats(LPGUID pGUID, IEnumFormat* pCallback)
	{
		LPDIRECTSOUNDCAPTURE pDSCapture = NULL;
		LPDIRECTSOUNDCAPTUREBUFFER pDSBuffer = NULL;

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

	HRESULT CCaptureDevice::SetCaptureDevice(const LPGUID pGUID)
	{
		m_pGUID = pGUID;
		return NOERROR;
	}

	HRESULT CCaptureDevice::SetCaptureFormat(const WAVEFORMATEX& wfex)
	{
		WAVEFORMATEX wfx = wfex;
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		LPDIRECTSOUNDCAPTURE pCapture = NULL;
		LPDIRECTSOUNDCAPTUREBUFFER pBuffer = NULL;
		HRESULT hr = DirectSoundCaptureCreate(m_pGUID, &pCapture, NULL);

		if (SUCCEEDED(hr))
		{
			DSCBUFFERDESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.dwSize = sizeof(desc);
			desc.dwBufferBytes = wfx.nSamplesPerSec * (wfx.nChannels * wfx.wBitsPerSample / 8);
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

	HRESULT CCaptureDevice::Open(ISampleCallback* pCallback)
	{
		if (m_bStarted)
		{
			Close();
		}
		m_pSampleCallback = pCallback;
		return NOERROR;
	}

	HRESULT CCaptureDevice::Start(void)
	{
		if (m_bStarted) return S_FALSE;

		HRESULT hr = S_OK;
		BOOL bRet = FALSE;

		do
		{
			hr = DirectSoundCaptureCreate(m_pGUID, &m_pDSCapture, NULL);
			if (FAILED(hr)) break;

			hr = CreateCaptureBuffer(m_wfex);
			if (FAILED(hr)) break;

			//create work thread
			DWORD dwThreadId = 0;
			m_hThread = CreateThread(NULL, 0, FetchThreadProxy, this, 0, &dwThreadId);
			hr = (m_hThread == NULL) ? E_FAIL : S_OK;
			if (FAILED(hr)) break;

			HRESULT hr = m_pDSBuffer->Start(DSCBSTART_LOOPING);
			if (FAILED(hr)) break;

			bRet = TRUE;
		} while (0);

		if (!bRet)
		{
			Stop();
		}

		m_bStarted = bRet;
		return hr;
	}

	HRESULT CCaptureDevice::Stop(void)
	{
		if (m_hThread)
		{
			SetEvent(m_hEventStop);
			WaitForSingleObject(m_hThread, INFINITE);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		SafeRelease(&m_pDSNotify);
		SafeRelease(&m_pDSBuffer);
		SafeRelease(&m_pDSCapture);
		m_bStarted = FALSE;
		return NOERROR;
	}

	HRESULT CCaptureDevice::Close(void)
	{
		m_dwNextCaptureOffset = 0;
		return Stop();
	}

	HRESULT CCaptureDevice::CreateCaptureBuffer(WAVEFORMATEX& wfex)
	{
		HRESULT hr;
		DSCBUFFERDESC desc;

		m_dwNotifySize = MAX(1024, m_wfex.nAvgBytesPerSec / 173);
		m_dwNotifySize -= m_dwNotifySize % m_wfex.nBlockAlign;
		m_dwCaptureBufferSize = m_dwNotifySize * NOTIFYS;

		ZeroMemory(&desc, sizeof(desc));
		desc.dwSize = sizeof(desc);
		desc.dwBufferBytes = m_dwCaptureBufferSize;
		desc.lpwfxFormat = &m_wfex;

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
	
	HRESULT CCaptureDevice::CaptureData(void)
	{
		HRESULT hr;
		VOID*   pbCaptureData = NULL;
		DWORD   dwCaptureLength;
		VOID*   pbCaptureData2 = NULL;
		DWORD   dwCaptureLength2;
		DWORD   dwReadPos;
		DWORD   dwCapturePos;
		LONG lLockSize;

		if (NULL == m_pDSBuffer) return E_POINTER;

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

		if (lLockSize == 0) return S_FALSE;

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

		// Write the data into the wav file
		if (m_pSampleCallback)
		{
			m_pSampleCallback->OnReceiveSamples(pbCaptureData
				, dwCaptureLength
				, m_wfex.nChannels
				, m_wfex.wBitsPerSample
				, m_wfex.nSamplesPerSec);
		}

		// Move the capture offset along
		m_dwNextCaptureOffset += dwCaptureLength;
		m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

		if (pbCaptureData2 != NULL)
		{
			// Write the data into the wav file
			if (m_pSampleCallback)
			{
				m_pSampleCallback->OnReceiveSamples(pbCaptureData2
					, dwCaptureLength2
					, m_wfex.nChannels
					, m_wfex.wBitsPerSample
					, m_wfex.nSamplesPerSec);
			}

			// Move the capture offset along
			m_dwNextCaptureOffset += dwCaptureLength2;
			m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
		}

		// Unlock the capture buffer
		hr = m_pDSBuffer->Unlock(pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);
		if (FAILED(hr)) return hr;

		return NOERROR;
	}

	DWORD CCaptureDevice::FetchSampleProc(void)
	{
		if (NULL == m_pDSBuffer) return 1;
		HANDLE hEvents[2] = { m_hEventNotify, m_hEventStop};

		while (1)
		{
			DWORD dwRet = ::WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
			//stop thread
			if (dwRet == WAIT_OBJECT_0 + 1)
			{
				break;
			}
			//record capture data
			if (dwRet == WAIT_OBJECT_0 + 0)
			{
				CaptureData();
				::ResetEvent(m_hEventNotify);
			}
		}

		return 0;
	}

	DWORD CCaptureDevice::FetchThreadProxy(LPVOID pParam)
	{
		return static_cast<CCaptureDevice*>(pParam)->FetchSampleProc();
	}
}


