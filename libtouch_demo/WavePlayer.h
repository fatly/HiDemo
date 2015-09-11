#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include "Defines.h"

#define PCM_COUNT	8
#define BUFFER_SIZE	32768

class CWavePlayer
{
public:
	CWavePlayer(int nChannels, int nSampleRate, int nBitPerSample);
	virtual ~CWavePlayer(void);
public:
	bool Play(char* pData, int nSize);
private:
	bool Init(void);
	void SetupFormat(int nChannels, int nSampleRate, int nBitPerSample);
	bool Abort(void);
	void Cleanup(void);
protected:
	struct PCM
	{
		PCM(void)
		{
			hWaveOut = NULL;
			memset(&header, 0, sizeof(header));
			header.dwFlags |= WHDR_DONE;
		}

		DWORD Prepare(HWAVEOUT hOut, uint nCount)
		{
			Release();
			memset(&header, 0, sizeof(header));

			header.lpData = data;
			header.dwBufferLength = nCount;
			header.dwUser = (DWORD)this;
			hWaveOut = hOut;

			return waveOutPrepareHeader(hWaveOut, &header, sizeof(header));
		}

		DWORD Release(void)
		{
			DWORD err = MMSYSERR_NOERROR;
			if (hWaveOut != NULL) {
				err = waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));
				if (err == WAVERR_STILLPLAYING) {
					return err;
				}
				hWaveOut = NULL;
			}

			header.dwFlags |= WHDR_DONE;
			return err;
		}

		char data[BUFFER_SIZE];
		WAVEHDR header;
		HWAVEOUT hWaveOut;
	};
protected:
	int m_nChannels;
	int m_nSampleRate;
	int m_nBitPerSample;
	WAVEFORMATEX m_WaveFormat;
	HWAVEOUT m_hWaveOut; 
	HANDLE m_hEventDone;
	HANDLE m_hMutex;
	int m_nBufferIndex;
	PCM m_PCMBuffer[PCM_COUNT];
};

