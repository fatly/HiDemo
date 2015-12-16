#include "stdafx.h"
#include "AudioRender.h"

namespace e
{
#define DEFINE_GUID2(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	DEFINE_GUID2(CLSID_AudioInputFilter,
		0x0005539a, 0xdb61, 0x4309, 0xaf, 0xa1, 0x35, 0x78, 0xe9, 0x00, 0x00, 0x34);

	inline void WriteMessage(const TCHAR* format,...)
	{
		va_list argptr;
		va_start(argptr, format);
		String msg = String::Format(format, argptr);
		va_end(argptr);
		OutputDebugString(msg.c_str());
		OutputDebugString(_T("\n"));
	}

	CAudioRender::CAudioRender(__in_opt LPCTSTR pName, __inout_opt LPUNKNOWN pUnk, __inout HRESULT *phr)
		: CBaseRenderer(CLSID_AudioInputFilter, pName, pUnk, phr)
	{

	}

	CAudioRender::~CAudioRender(void)
	{

	}

	HRESULT CAudioRender::CheckMediaType(const CMediaType* pmt)
	{
		if (!(IsEqualGUID(pmt->majortype, MEDIATYPE_Audio) && 
			IsEqualGUID(pmt->subtype, MEDIASUBTYPE_PCM) && 
			IsEqualGUID(pmt->formattype, FORMAT_WaveFormatEx)))
		{
			return E_FAIL;
		}

		WAVEFORMATEX* pwfx = (WAVEFORMATEX*)pmt->pbFormat;
		if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
		{
			return E_FAIL;
		}

		if (!(pwfx->cbSize == 0
			&& pwfx->nChannels == 2
			&& pwfx->wBitsPerSample == 16
			&& pwfx->nSamplesPerSec == 44100
			&& pwfx->nBlockAlign == (pwfx->nChannels * pwfx->wBitsPerSample) / 8
			&& pwfx->nAvgBytesPerSec == pwfx->nSamplesPerSec * pwfx->nBlockAlign
			))
		{
			return E_FAIL;
		}

		m_mt.Set(*pmt);

		WriteMessage(_T("AudioRender::CheckMediaType\n"));
		return S_OK;
	}

	HRESULT CAudioRender::DoRenderSample(IMediaSample* pmt)
	{
		CheckPointer(pmt, E_POINTER);
		LONG lSampleSize = pmt->GetActualDataLength();

		static int iCount = 0;
		WriteMessage(_T("AudioRender::DoRenderSample:%d,%d"), iCount++, lSampleSize);
		return S_OK;
	}
}
