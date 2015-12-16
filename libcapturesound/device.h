#pragma once
#include <InitGuid.h>
#include <DSound.h>
#pragma comment(lib, "dsound.lib")
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

	class ISampleCallback
	{
	public:
		virtual void OnReceiveSamples(void* pSamples, int nSize, int nChannels, int nBitsPerSamle, int nSamplesPerSec) = 0;
		virtual void OnReportError(int nCode) = 0;
	};

	class CCaptureDevice
	{
	public:
		CCaptureDevice(void);
		virtual ~CCaptureDevice(void);
	public:
		HRESULT GetDevices(IEnumDevice* pCallback);
		HRESULT GetFormats(LPGUID pGUID, IEnumFormat* pCallback);
		HRESULT SetCaptureDevice(const LPGUID pGUID);
		HRESULT SetCaptureFormat(const WAVEFORMATEX& wfex);
		HRESULT Open(ISampleCallback* pCallback);
		HRESULT Start(void);
		HRESULT Stop(void);
		HRESULT Close(void);
	protected:
		HRESULT CreateCaptureBuffer(WAVEFORMATEX& wfex);
		HRESULT CaptureData(void);
		DWORD FetchSampleProc(void);
		static DWORD WINAPI FetchThreadProxy(LPVOID pParam);
	protected:
		LPGUID m_pGUID;
		LPDIRECTSOUNDNOTIFY  m_pDSNotify;
		LPDIRECTSOUNDCAPTURE	 m_pDSCapture;
		LPDIRECTSOUNDCAPTUREBUFFER m_pDSBuffer;
		DSBPOSITIONNOTIFY  m_aPosNotify[NOTIFYS + 1];
		ISampleCallback* m_pSampleCallback;
		WAVEFORMATEX m_wfex;
		HANDLE m_hThread;
		HANDLE m_hEventStop;
		HANDLE m_hEventNotify;
		BOOL m_bStarted;
		DWORD m_dwNotifySize;
		DWORD  m_dwCaptureBufferSize;
		DWORD  m_dwNextCaptureOffset;
	};
}
