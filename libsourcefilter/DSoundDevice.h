#pragma once
#include <dsound.h>
#include <vector>
#include "Constants.h"

namespace e
{
#define NOTIFYS 16

	class IEnumDevice
	{
	public:
		virtual void OnAddDevice(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName) = 0;
	};

	class IEnumFormat
	{
	public:
		virtual void OnAddFormat(WAVEFORMATEX& wfex) = 0;
	};

	typedef struct TDeviceItem
	{
		LPGUID pGUID;
		TCHAR szName[MAX_PATH];
		TDeviceItem(LPGUID pGUID, LPCTSTR pszName)
		{
			this->pGUID = pGUID;
			_tcscpy_s(szName, pszName);
		}
	}DEVICEITEM;

	class CDSoundDevice
	{
	public:
		CDSoundDevice(CCritSec* pStateLock);
		virtual ~CDSoundDevice(void);
	public:
		HRESULT GetDevices(IEnumDevice* pCallback);
		//此接口调用会很慢很耗时
		HRESULT GetFormats(LPGUID pGUID, IEnumFormat* pCallback);
		HRESULT SetCaptureDevice(const LPGUID pGUID);
		HRESULT SetCaptureDevice(LPCTSTR pszDeviceName);
		HRESULT SetCaptureFormat(const WAVEFORMATEX& wfex);
		HRESULT SetCaptureSampleSize(int nBufferSize = kMaxSampleSize);
		HRESULT GetPCMFormat(WAVEFORMATEX* pwfex);
		HRESULT GetPCMBuffer(void* pBuffer, int nMaxSize, int* nRead);
		HRESULT OnThreadCreate(void);
		HRESULT OnThreadStartPlay(void);
		HRESULT OnThreadDestory(void);
	protected:
		HRESULT CreateCaptureBuffer(WAVEFORMATEX& wfex);
	protected:
		CCritSec* m_pStateLock;
		LPGUID m_pDeviceGUID;
		LPDIRECTSOUNDNOTIFY  m_pDSNotify;
		LPDIRECTSOUNDCAPTURE	 m_pDSCapture;
		LPDIRECTSOUNDCAPTUREBUFFER m_pDSBuffer;
		DSBPOSITIONNOTIFY  m_aPosNotify[NOTIFYS + 1];
		WAVEFORMATEX m_wfex;
		HANDLE m_hEventNotify;
		DWORD m_dwNotifySize;
		DWORD m_dwCaptureSampleSize;
		DWORD  m_dwCaptureBufferSize;
		DWORD  m_dwNextCaptureOffset;

		std::vector<DEVICEITEM> m_DeviceList;
	};
}

