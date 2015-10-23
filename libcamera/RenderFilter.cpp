#include "RenderFilter.h"
#include "FrameCtrl.h"

#define DEFINE_GUID1(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_GUID1(CLSID_VideoInputFilter,
	0xa4a504b, 0x2339, 0x4291, 0x80, 0x5d, 0xc3, 0x1b, 0x83, 0x7f, 0xf4, 0x8a);

namespace e
{
	RenderFilter::RenderFilter(__in_opt LPCTSTR filterName
		, __inout_opt IUnknown* unknown
		, __out_opt HRESULT* result)
		:CBaseRenderer(CLSID_VideoInputFilter, filterName, unknown, result)
	{
		dataType = UNKNOWN;
		width = 0;
		height = 0;
		bitCount = 32;
		totalBytes = 0;
		rgbBuffer = 0;
		tmpBuffer = 0;
		startTime = -1;
		endTime = -1;
		isSuport = false;
		fnSampleHandle = 0;
		fnSampleParam = 0;
		ffscaler.SetAttribute(SWS_PF_BGR24, SWS_PF_RGBA, SWS_SA_BILINEAR);
		fps = new CFrameCtrl();
		fps->SetFramePerSecond(15);
	}

	RenderFilter::~RenderFilter()
	{
		if (rgbBuffer)
		{
			delete[] rgbBuffer;
			rgbBuffer = 0;
		}

		if (tmpBuffer)
		{
			delete[] tmpBuffer;
			tmpBuffer = 0;
		}

		if (fps)
		{
			delete fps;
			fps = NULL;
		}
	}

	HRESULT RenderFilter::CheckMediaType(const CMediaType* mt)
	{
		if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_IYUV))
		{
			dataType = IYUV;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_RGB24))
		{
			ffscaler.SetAttribute(SWS_PF_RGB24, SWS_PF_RGBA, SWS_SA_BILINEAR);
			dataType = RGB24;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_RGB32))
		{
			dataType = RGB32;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_YV12))
		{
			dataType = YV12;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_UYVY) || ::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_HDYC))
		{
			ffscaler.SetAttribute((PicFormat)PIX_FMT_UYVY422, SWS_PF_RGBA, SWS_SA_BILINEAR);
			dataType = UYVY;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_YUY2))
		{
			ffscaler.SetAttribute(SWS_PF_YUV422, SWS_PF_BGRA, SWS_SA_BILINEAR);
			dataType = YUY2;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_YVYU))
		{
			dataType = YVYU;
		}
		else if (::IsEqualCLSID(mt->subtype, MEDIASUBTYPE_I420))
		{
			dataType = I420;
		}
		else
		{
			dataType = UNKNOWN;
			isSuport = false;
		}

		width = ((VIDEOINFOHEADER*)mt->pbFormat)->bmiHeader.biWidth;
		height = ((VIDEOINFOHEADER*)mt->pbFormat)->bmiHeader.biHeight;
		//bitCount = ((VIDEOINFOHEADER*)mt->pbFormat)->bmiHeader.biBitCount;

		SAFE_DELETE(rgbBuffer);
		int size = width * height * 4;
		rgbBuffer = new uchar[size];
		memset(rgbBuffer, 0, size);

		SAFE_DELETE(tmpBuffer);
		tmpBuffer = new uchar[size];
		memset(tmpBuffer, 0, size);

		return S_OK;
	}

	HRESULT RenderFilter::DoRenderSample(IMediaSample* sample)
	{
		if (fps->Step() != CFrameCtrl::STATE_RENDER)
		{
			return S_OK;
		}

		BYTE* buffer = 0;
		sample->GetPointer(&buffer);
		sample->GetMediaTime(&startTime, &endTime);
		long size = sample->GetSize();

		if (fnSampleHandle)
		{
			int lineBytes = WIDTHBYTES(KOutputVideoWidth * KOutputVideoBitCount);
			if (dataType == YUY2)
			{				
				ffscaler.Scale(buffer, width, height, width * 2, rgbBuffer, KOutputVideoWidth, KOutputVideoHeight, lineBytes);
				buffer = rgbBuffer;
			}

			totalBytes = KOutputVideoHeight * lineBytes;
			fnSampleHandle(fnSampleParam, buffer, totalBytes, KOutputVideoWidth, KOutputVideoHeight, KOutputVideoBitCount);
		}
		else
		{
			totalBytes = size;
			memcpy(rgbBuffer, buffer, totalBytes);
		}

		return S_OK;
	}

	void RenderFilter::GetSize(int & width, int & height, int &bitCount) const
	{
		width = this->width;
		height = this->height;
		bitCount = KOutputVideoBitCount;
	}

	void RenderFilter::GetData(DataType & type, uchar* buffer, int & width, int & height, int &bitCount) const
	{
		type = this->dataType;
		width = this->width;
		height = this->height;
		bitCount = KOutputVideoBitCount;
		memcpy(buffer, rgbBuffer, totalBytes);
	}
}

