#include "WavePlayer.h"
#include <assert.h>


CWavePlayer::CWavePlayer(int nChannels, int nSampleRate, int nBitPerSample)
{
	m_hWaveOut = NULL;
	m_hEventDone = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_hEventDone);
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	assert(m_hMutex);
	m_nBufferIndex = 0;

	SetupFormat(nChannels, nSampleRate, nBitPerSample);
	bool bRet = Init();
	assert(bRet);
}

CWavePlayer::~CWavePlayer()
{
	Cleanup();
}

void CWavePlayer::SetupFormat(int nChannels, int nSampleRate, int nBitPerSample)
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nChannels = nChannels;
	m_WaveFormat.wBitsPerSample = nBitPerSample;
	m_WaveFormat.nSamplesPerSec = (DWORD)(nSampleRate * 0.979);
	m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels * m_WaveFormat.wBitsPerSample / 8;
	m_WaveFormat.nAvgBytesPerSec = (DWORD)(m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign * 0.979);
	m_WaveFormat.cbSize = sizeof(m_WaveFormat);

	m_nChannels = nChannels;
	m_nSampleRate = nSampleRate;
	m_nBitPerSample = nBitPerSample;
}

bool CWavePlayer::Init(void)
{
	MMRESULT mResult = waveOutOpen(NULL
		, WAVE_MAPPER
		, &m_WaveFormat
		, NULL
		, NULL
		, WAVE_FORMAT_QUERY);

	if (mResult != MMSYSERR_NOERROR) return false;

	mResult = waveOutOpen(&m_hWaveOut
		, WAVE_MAPPER
		, &m_WaveFormat
		, (DWORD_PTR)m_hEventDone
		, NULL
		, CALLBACK_EVENT);

	if (mResult != MMSYSERR_NOERROR) return false;
	//reset
	m_nBufferIndex = 0;

	return true;
}

void CWavePlayer::Cleanup(void)
{
	if (m_hWaveOut == NULL) return;

	Abort();

	if (m_hWaveOut != NULL)
	{
		while (waveOutClose(m_hWaveOut) == WAVERR_STILLPLAYING) 
		{
			waveOutReset(m_hWaveOut);
		}

		m_hWaveOut = NULL;
	}

	Abort();

	if (m_hEventDone)
	{
		CloseHandle(m_hEventDone);
		m_hEventDone = NULL;
	}

	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}

bool CWavePlayer::Abort(void)
{
	DWORD dwError = MMSYSERR_NOERROR;

	if (m_hWaveOut != NULL)
	{
		dwError = waveOutReset(m_hWaveOut);
	}
	::WaitForSingleObject(m_hMutex, INFINITE);

	if (m_hWaveOut != NULL)
	{
		for (int i = 0; i < PCM_COUNT; i++)
		{
			while (m_PCMBuffer[i].Release() == WAVERR_STILLPLAYING)
			{
				if (m_hWaveOut != NULL)
				{
					waveOutReset(m_hWaveOut);
				}
			}
		}
	}

	::ReleaseMutex(m_hMutex);
	m_nBufferIndex = 0;
	// Signal any threads waiting on this event, they should then check
	// the bufferByteOffset variable for an abort.
	::SetEvent(m_hEventDone);

	return (dwError != MMSYSERR_NOERROR);
}

bool CWavePlayer::Play(char* pData, int nSize)
{
	assert(m_hWaveOut);
	if (m_hWaveOut == NULL) return false;

	char* ptr = pData;
	::WaitForSingleObject(m_hMutex, INFINITE);

	DWORD dwError = MMSYSERR_NOERROR;
	while (nSize > 0) 
	{
		DWORD flags = m_PCMBuffer[m_nBufferIndex].header.dwFlags;

		while ((flags & WHDR_DONE) == 0)
		{
			::ReleaseMutex(m_hMutex);
			if (::WaitForSingleObject(m_hEventDone, INFINITE) != WAIT_OBJECT_0)
			{
				return false;
			}

			::WaitForSingleObject(m_hMutex, INFINITE);
			flags = m_PCMBuffer[m_nBufferIndex].header.dwFlags;
		}

		dwError = m_PCMBuffer[m_nBufferIndex].Prepare(m_hWaveOut, nSize);
		if (dwError != MMSYSERR_NOERROR) break;

		assert(nSize <= BUFFER_SIZE);
		memcpy(m_PCMBuffer[m_nBufferIndex].data, ptr, nSize);
		dwError = waveOutWrite(m_hWaveOut, &m_PCMBuffer[m_nBufferIndex].header, sizeof(WAVEHDR));
		if (dwError != MMSYSERR_NOERROR) break;
		
		m_nBufferIndex = (m_nBufferIndex + 1) % PCM_COUNT;
		break;
	}

	::ReleaseMutex(m_hMutex);
	return 0;
}