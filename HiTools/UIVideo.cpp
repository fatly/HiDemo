#include "stdafx.h"
#include "UIVideo.h"

namespace DuiLib
{
	CVideoUI::CVideoUI()
	{
		m_hBitmap = NULL;
		m_pBits = NULL;
		m_dwSize = 0;
		m_nWidth = 0;
		m_nHeight = 0;
		memset(&m_rcBitmap, 0, sizeof(m_rcBitmap));
		memset(&m_rcCorners, 0, sizeof(m_rcCorners));
#if _THREAD_SAFE
		::InitializeCriticalSection(&m_csLock);
#endif
	}

	CVideoUI::~CVideoUI()
	{
		if (m_hBitmap != NULL)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}

#if _THREAD_SAFE
		::DeleteCriticalSection(&m_csLock);
#endif
	}

	LPCTSTR CVideoUI::GetClass(void) const
	{
		return _T("VideoUI");
	}

	LPVOID CVideoUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Video")) == 0)
		{
			return static_cast<CVideoUI*>(this);
		}
		else
		{
			return CControlUI::GetInterface(pstrName);
		}
	}

	bool CVideoUI::Init(int nWidth, int nHeight)
	{
		int nBitCount = 32;
		DWORD dwSizeImage = WIDTHBYTES(nWidth * nBitCount) * nHeight;

		BITMAPINFO bmi;
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nWidth;
		bmi.bmiHeader.biHeight = -nHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = nBitCount;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = dwSizeImage;

		BYTE* pBits = NULL;
		HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
		if (hBitmap == INVALID_HANDLE_VALUE) return false;

		memset(pBits, 0, dwSizeImage);

		m_pBits = pBits;
		m_dwSize = dwSizeImage;
		if (m_hBitmap != NULL)
		{
			::DeleteObject(m_hBitmap);
		}
		m_hBitmap = hBitmap;
		m_nWidth = nWidth;
		m_nHeight = nHeight;

		m_rcBitmap.left = 0;
		m_rcBitmap.top = 0;
		m_rcBitmap.right = nWidth;
		m_rcBitmap.bottom = nHeight;

		return true;
	}

	void CVideoUI::SetVideo(BYTE* pData, int nWidth, int nHeight, int nBitCount)
	{
		ASSERT(pData);
		ASSERT(nBitCount == 24 || nBitCount == 32);
		
		if (pData == NULL) return;

		if (nWidth != m_nWidth || nHeight != m_nHeight)
		{
			Init(nWidth, nHeight);
			ASSERT(m_hBitmap);
		}

		if (m_hBitmap == NULL) return;

#if _THREAD_SAFE
		::EnterCriticalSection(&m_csLock);
#endif
		if (nBitCount == 24)
		{
			int nSrcLineByte = WIDTHBYTES(nWidth * 24);
			int nDstLineByte = WIDTHBYTES(nWidth * 32);

			for (int y = 0; y < nHeight; y++)
			{
				BYTE* pSrc = pData + nSrcLineByte * y;
				BYTE* pDst = m_pBits + nDstLineByte * y;
				for (int x = 0; x < nWidth; x++)
				{
					pDst[0] = pSrc[0];
					pDst[1] = pSrc[1];
					pDst[2] = pSrc[2];
					pDst[3] = 255;

					pSrc += 3;
					pDst += 4;
				}
			}
		}
		else if (nBitCount == 32)
		{
			memcpy(m_pBits, pData, nWidth * nHeight * 4);
		}
#if _THREAD_SAFE
		::LeaveCriticalSection(&m_csLock);
#endif
	}

	void CVideoUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_KEYDOWN)
		{
			static bool flag = false;
			if (!flag)
			{
				Start();
			}
			else
			{
				Stop();
			}
			flag = !flag;
		}

		return CControlUI::DoEvent(event);
	}

	void CVideoUI::DoPaint(HDC hDC, const RECT & rcPaint)
	{
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem))
		{
			return;
		}

		if (m_hBitmap != NULL)
		{
			int x, y, w, h;
			int w0 = m_rcItem.right - m_rcItem.left;
			int h0 = m_rcItem.bottom - m_rcItem.top;
			int w1 = m_rcBitmap.right - m_rcBitmap.left;
			int h1 = m_rcBitmap.bottom - m_rcBitmap.top;

			if (w0 > w1)
			{
				x = m_rcItem.left + (w0 - w1) / 2;
				w = w1;
			}
			else
			{
				x = m_rcItem.left;
				w = w0;
			}

			if (h0 > h1)
			{
				y = m_rcItem.top + (h0 - h1) / 2;
				h = h1;
			}
			else
			{
				y = m_rcItem.top;
				h = h0;
			}

			RECT rcItem = {x, y, x+w, y+h};
			
			CRenderEngine::DrawColor(hDC, m_rcItem, 0xFF000000);
#if _THREAD_SAFE
			::EnterCriticalSection(&m_csLock);
			CRenderEngine::DrawImage(hDC, m_hBitmap, rcItem, rcPaint, m_rcBitmap, m_rcCorners, false, 255);
			::LeaveCriticalSection(&m_csLock);
#else
			CRenderEngine::DrawImage(hDC, m_hBitmap, m_rcItem, rcPaint, m_rcBitmap, m_rcCorners, false, 255);
#endif
		}
		else
		{
			CControlUI::DoPaint(hDC, rcPaint);
		}
	}

	void CVideoUI::FormatHandle(TCHAR* type, int width, int height)
	{
		//TODO:

		//Camera::FormatHandle(type, width, height);
	}

	void CVideoUI::SampleHandle(uchar* buffer, int width, int height, int bitCount)
	{
		SetVideo(buffer, width, height, bitCount);
		NeedUpdate();
	}
}
