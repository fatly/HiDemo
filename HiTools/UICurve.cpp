#include "stdafx.h"
#include "UICurve.h"
#include "Curve.h"

namespace DuiLib
{
	CCurveUI::CCurveUI()
	{
		m_hBitmap = NULL;
		m_pCurves = new e::CurvesConfig(17, 256);
		memset(&m_rcBitmap, 0, sizeof(m_rcBitmap));
		memset(&m_rcCorners, 0, sizeof(m_rcCorners));
		memset(&m_hPen, 0, sizeof(HPEN) * MAX_PEN_SIZE);
		Init(256, 256);
		InitPen(6);
	}

	CCurveUI::~CCurveUI()
	{
		Clear();
	}

	LPCTSTR CCurveUI::GetClass(void) const
	{
		return _T("CurveUI");
	}

	LPVOID CCurveUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Curve")) == 0)
		{
			return static_cast<CCurveUI*>(this);
		}
		else
		{
			return CControlUI::GetInterface(pstrName);
		}
	}

	bool CCurveUI::Init(int nWidth, int nHeight)
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
		memset(pBits, 255, dwSizeImage);

		if (m_hBitmap != NULL)	::DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;

		m_rcBitmap.left = 0;
		m_rcBitmap.top = 0;
		m_rcBitmap.right = nWidth;
		m_rcBitmap.bottom = nHeight;

		UpdateUI();
		return true;
	}

	bool CCurveUI::InitPen(int nCount)
	{
		limit(nCount, 6, MAX_PEN_SIZE);

		COLORREF colors[] = {
			RGB(45, 45, 45),
			RGB(255, 0, 0),
			RGB(0, 255, 0),
			RGB(0, 0, 255),
			RGB(240, 170, 30),
			RGB(210, 210, 210)
		};

		for (int i = 0; i <nCount; i++)
		{
			m_hPen[i] = ::CreatePen(PS_SOLID, 1, colors[i]);
			assert(m_hPen[i] != NULL);
		}

		return true;
	}

	void CCurveUI::DoEvent(TEventUI& event)
	{
		TCHAR szText[64] = { 0 };
		_stprintf_s(szText, _T("CCurveUI::DoEvent()->%d\n"), event.Type);
		OutputDebugString(szText);

		UpdateUI();

		CControlUI::DoEvent(event);
	}

	void CCurveUI::DoPaint(HDC hDC, const RECT& rcPaint)
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

			RECT rcItem = { x, y, x + w, y + h };
			CRenderEngine::DrawColor(hDC, m_rcItem, 0xFFFFFF00);
			CRenderEngine::DrawImage(hDC, m_hBitmap, rcItem, rcPaint, m_rcBitmap, m_rcCorners, false, 255);
		}
		else
		{
			CControlUI::DoPaint(hDC, rcPaint);
		}
	}

	void CCurveUI::UpdateUI(void)
	{
		HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		HDC  hMemDC = ::CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);
		::FillRect(hMemDC, &m_rcBitmap, hBrush);

		int w = m_rcBitmap.right - m_rcBitmap.left;
		int h = m_rcBitmap.bottom - m_rcBitmap.top;
		int nChannel = m_pCurves->GetSelectChannel();
		//画没选中的B线
		for (int c = CURVE_CHANNEL_A; c >= CURVE_CHANNEL_C; c--)
		{
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[c]);
			Curve* curve = m_pCurves->GetCurve(c);
			if (curve->IsIdentity() || c == nChannel) continue;

			int y = h - curve->GetSample(0);
			if (y >= h) y -= 1;
			::MoveToEx(hMemDC, 0, y, NULL);

			for (int x = 1; x < curve->GetSampleCount()&&x<w; x++)
			{
				y = h - curve->GetSample(x);
				if (y >= h) y -= 1;
				::LineTo(hMemDC, x, y);
			}

			::SelectObject(hMemDC, hOldPen);
		}

		//画选中的B线
		HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[nChannel]);
		Curve* curve = m_pCurves->GetCurve(nChannel);

		int y = h - curve->GetSample(0);
		if (y >= h) y -= 1;
		::MoveToEx(hMemDC, 0, y, NULL);

		for (int x = 1; x < curve->GetSampleCount() && x < w; x++)
		{
			y = h - curve->GetSample(x);
			if (y >= h) y -= 1;
			::LineTo(hMemDC, x, y);
		}
		::SelectObject(hMemDC, hOldPen);
	
		int x0 = m_rcBitmap.left;
		int y0 = m_rcBitmap.top;
		int x1 = m_rcBitmap.right-1;
		int y1 = m_rcBitmap.bottom-1;
		hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[0]);
		::MoveToEx(hMemDC, x0, y0, NULL);
		::LineTo(hMemDC, x1, y0);
		::LineTo(hMemDC, x1, y1);
		::LineTo(hMemDC, x0, y1);
		::LineTo(hMemDC, x0, y0);
		::SelectObject(hMemDC, hOldPen);

		::SelectObject(hMemDC, hOldBitmap);
		::DeleteObject(hBrush);
		::DeleteDC(hMemDC);
	}

	void CCurveUI::Clear(void)
	{
		if (m_pCurves)
		{
			delete m_pCurves;
			m_pCurves = 0;
		}

		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}

		for (int i = 0; i < MAX_PEN_SIZE; i++)
		{
			if (m_hPen[i] != NULL)
			{
				::DeleteObject(m_hPen[i]);
				m_hPen[i] = NULL;
			}
		}
	}
}
