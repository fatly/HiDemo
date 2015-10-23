#ifndef __E_RENDERFITLER_H__
#define __E_RENDERFILTER_H__

#include <streams.h>
#include "FFScale.h"
#include "libcamdefs.h"

namespace e
{
#define DEFINE_GUID1(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
		= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	DEFINE_GUID1(MEDIASUBTYPE_I420,
		0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38,
		0x9B, 0x71);

	DEFINE_GUID1(PHYSICAL_CAM,
		0x17CCA71B, 0xECD7, 0x11D0, 0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22,
		0x31, 0x96);

	DEFINE_GUID1(MEDIASUBTYPE_HDYC,
		0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38,
		0x9B, 0x71);

	enum DataType{ RGB24, IYUV, RGB32, YV12, IMC1, IMC2, IMC3, IMC4, UYVY, YUY2, YVYU, I420, UNKNOWN };

	class CFrameCtrl;
	class RenderFilter : public CBaseRenderer
	{
	public:
		RenderFilter(__in_opt LPCTSTR filterName
			, __inout_opt IUnknown* unknown
			, __out_opt HRESULT* result);

		virtual ~RenderFilter(void);

		virtual void OnReceiveFirstSample(IMediaSample* sample){};
		virtual HRESULT CheckMediaType(const CMediaType* mt);
		virtual HRESULT DoRenderSample(IMediaSample* sample);

		void GetSize(int &width, int &height, int &bitCount) const;
		void GetData(DataType &type, uchar* buffer, int &width, int &height, int &bitCount) const;

		void SetSampleHandle(SampleHanlde fnHandle, void* param)
		{
			fnSampleHandle = fnHandle;
			fnSampleParam = param;
		}
		
	private:

		CFFScale ffscaler;

		uchar*	rgbBuffer;
		uchar*  tmpBuffer;

		REFERENCE_TIME startTime;
		REFERENCE_TIME endTime;
		DataType dataType;

		int width;
		int height;
		int bitCount;
		int totalBytes;
		bool isSuport;

		SampleHanlde fnSampleHandle;
		void* fnSampleParam;

		CFrameCtrl* fps;
	};
}

#endif
