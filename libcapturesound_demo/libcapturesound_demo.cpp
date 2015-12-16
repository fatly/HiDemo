// libcapturesound_demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <locale.h>
#include "dsutil.h"

class CTest 
	: public IEnumDevice
	, public IEnumFormat
	, public ISampleCallback
{
public:
	CTest(void)
	{
		wfx.wFormatTag = 1;
		wfx.nChannels = 2;
		wfx.wBitsPerSample = 16;
		wfx.nSamplesPerSec = 44100;
		wfx.nBlockAlign = 4;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

		device = new CCaptureDevice();

		file = new CWaveFile();
		file->Open(_T("f:\\test.wav"), &wfx, WAVEFILE_WRITE);
	}
	~CTest()
	{
		file->Close();
		delete device;
	}

	virtual void OnAddDevice(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName)
	{
// 		if (lpGUID)
// 			_tprintf(_T("guid=%u.%u.%u.%u,desc=%s, name=%s\n"), lpGUID->Data1, lpGUID->Data2, lpGUID->Data3, lpGUID->Data4, lpszDesc, lpszDrvName);
// 		else
// 			_tprintf(_T("desc=%s,name=%s\n"), lpszDesc, lpszDrvName);
		static int i = 0;
		if (i++ == 1)
		{
			this->pGUID = lpGUID;
		}
		_tprintf(_T("desc=%s,name=%s\n"), lpszDesc, lpszDrvName);
	}

	virtual void OnAddFormat(WAVEFORMATEX& wfex)
	{
		_tprintf(_T("channels =%d, bits=%d, srate=%d\n"), wfex.nChannels, wfex.wBitsPerSample, wfex.nSamplesPerSec);
	}

	virtual void OnReceiveSamples(void* pSamples, int nSize, int nChannels, int nBitsPerSamle, int nSamplesPerSec)
	{
		static int i = 0;
		UINT nBytes = 0;
		file->Write(nSize, (BYTE*)pSamples, &nBytes);
		if (i++ % 10 == 0)
			printf("[%d] samples:%d,%d\n", i++, nSize,nBytes);
	}

	virtual void OnReportError(int nCode)
	{

	}

	void GetDevices(void)
	{
		device->GetDevices(this);
	}

	void GetFormats(void)
	{
		device->GetFormats(NULL, this);
	}

	void SetFormat(void)
	{
		HRESULT hr = device->SetCaptureFormat(wfx);

		printf("set format %s\n", hr == S_OK ? "ok":"false");
	}

	void SetDevice(void)
	{
		HRESULT hr = device->SetCaptureDevice(pGUID);
		printf("set device %s\n", hr == S_OK ? "ok" : "false");
	}

	void Start(void)
	{
		HRESULT hr = device->Open(this);
		if (FAILED(hr))
		{
			printf("open device failed\n");
			return;
		}

		hr = device->Start();
		if (FAILED(hr))
		{
			printf("start device failed\n");
			return;
		}

		printf("start device ok\n");
	}

	void Stop(void)
	{
		HRESULT hr = device->Stop();
		if (FAILED(hr))
		{
			printf("stop device failed\n");
			return;
		}

		hr = device->Close();
		if (FAILED(hr))
		{
			printf("close device failed\n");
			return;
		}

		printf("close device ok\n");
	}

protected:
	LPGUID pGUID;
	CCaptureDevice* device;
	CWaveFile* file;
	WAVEFORMATEX wfx;
};

int _tmain(int argc, _TCHAR* argv[])
{
	::CoInitialize(NULL);
	setlocale(LC_ALL, "");
	CTest* tester = new CTest();

	tester->GetDevices();
//	tester->GetFormats();
	tester->SetDevice();
	tester->SetFormat();

	tester->Start();

	int cmd = 0;
	while (scanf_s("%d", &cmd))
	{
		if (cmd == 1)
		{
			tester->Stop();
		}
		else if (cmd == 2)
		{
			tester->Start();
		}
		else
		{
			break;
		}
	}
	//tester->Stop();

	delete tester;
	::CoUninitialize();
	system("pause");
	return 0;
}

