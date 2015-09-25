#include "stdafx.h"
#include "UICurve.h"
#include "CurvesConfig.h"

#define  MIN_DISTANCE 8

namespace DuiLib
{
	CSubCurveUI::CSubCurveUI()
	{
		m_hBitmap = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
		m_nPointCount = 9;
		m_bCapture = false;
		m_nSelectPoint = -1;
		m_nLeftMost = 0;
		m_nRightMost = 0;
		memset(&m_rcBitmap, 0, sizeof(m_rcBitmap));
		memset(&m_rcCorners, 0, sizeof(m_rcCorners));
		memset(&m_hPen, 0, sizeof(HPEN) * PEN_SIZE);
		InitPen(6);
		Init(256, 256);
		m_pCurvesConfig = new e::CurvesConfig(m_nPointCount, m_nWidth);
		assert(m_pCurvesConfig);
	}

	CSubCurveUI::CSubCurveUI(int nWidth, int nHeight)
	{
		m_hBitmap = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
		m_nPointCount = 9;
		m_bCapture = false;
		m_nSelectPoint = -1;
		m_nLeftMost = 0;
		m_nRightMost = 0;
		memset(&m_rcBitmap, 0, sizeof(m_rcBitmap));
		memset(&m_rcCorners, 0, sizeof(m_rcCorners));
		memset(&m_hPen, 0, sizeof(HPEN) * PEN_SIZE);
		InitPen(6);
		Init(nWidth, nHeight);
		m_pCurvesConfig = new e::CurvesConfig(m_nPointCount, m_nWidth);
		assert(m_pCurvesConfig);
	}

	CSubCurveUI::~CSubCurveUI()
	{
		Clear();
	}

	LPCTSTR CSubCurveUI::GetClass(void) const
	{
		return _T("SubCurveUI");
	}

	LPVOID CSubCurveUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("SubCurve")) == 0)
		{
			return static_cast<CSubCurveUI*>(this);
		}
		else
		{
			return CControlUI::GetInterface(pstrName);
		}
	}

	bool CSubCurveUI::Init(int nWidth, int nHeight)
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

		m_nWidth = nWidth;
		m_nHeight = nHeight;

		SetPos(m_rcBitmap, false);
		return true;
	}

	bool CSubCurveUI::InitPen(int nCount)
	{
		limit(nCount, 6, PEN_SIZE);

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
			if (m_hPen[i] == NULL) return false;
		}

		return true;
	}

	void CSubCurveUI::DoEvent(TEventUI& event)
	{
		TCHAR szText[64] = { 0 };
		_stprintf_s(szText, _T("CSubCurveUI::DoEvent()->%d\n"), event.Type);
		OutputDebugString(szText);

		if (event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			switch (event.Type)
			{
			case UIEVENT_BUTTONDOWN:
				OnLButtonDown(event.ptMouse);
				break;
			case UIEVENT_MOUSEMOVE:
				OnMouseMove(event.ptMouse);
				break;
			case UIEVENT_BUTTONUP:
				OnLButtonUp(event.ptMouse);
				break;
			default:
				break;
			}
		}

		CControlUI::DoEvent(event);
	}

	void CSubCurveUI::DoPaint(HDC hDC, const RECT& rcPaint)
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

	void CSubCurveUI::Update(void)
	{
		HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		HDC  hMemDC = ::CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);
		::FillRect(hMemDC, &m_rcBitmap, hBrush);

		int w = m_nWidth;
		int h = m_nHeight;
		int nChannel = m_pCurvesConfig->GetSelectIndex();
		
		{	//画没选中的B线
			for (int c = CURVE_CHANNEL_A; c >= CURVE_CHANNEL_C; c--)
			{
				HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[c]);
				SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(c);
				if (!curve->IsIdentity() || c == nChannel) continue;

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
			}
		}

		
		{	//画参考线
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[5]);
			SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(5);

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
		}

		{	//画选中的B线
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[nChannel]);
			SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(nChannel);

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
		}
	
		{	//画边框
			int x0 = m_rcBitmap.left;
			int y0 = m_rcBitmap.top;
			int x1 = m_rcBitmap.right - 1;
			int y1 = m_rcBitmap.bottom - 1;
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[0]);
			::MoveToEx(hMemDC, x0, y0, NULL);
			::LineTo(hMemDC, x1, y0);
			::LineTo(hMemDC, x1, y1);
			::LineTo(hMemDC, x0, y1);
			::LineTo(hMemDC, x0, y0);
			::SelectObject(hMemDC, hOldPen);
		}

		::SelectObject(hMemDC, hOldBitmap);
		::DeleteObject(hBrush);
		::DeleteDC(hMemDC);

		NeedUpdate();
	}

	void CSubCurveUI::OnLButtonDown(POINT &point)
	{
		point.x -= (m_rcItem.right - m_rcBitmap.right) / 2;
		point.y -= (m_rcItem.bottom - m_rcBitmap.bottom) / 2;

		int x, y;
		int w = (m_rcBitmap.right-m_rcBitmap.left) / (m_nPointCount - 1);
		int distance = MAXINT;
		int nSelectPoint = -1;
		SplineCurve* curve = m_pCurvesConfig->GetSelectSplineCurve();
		for (int i = 0; i < m_nPointCount; i++)
		{
			curve->GetPoint(i, x, y);
			if (abs(x - point.x) < distance)
			{
				distance = abs(x - point.x);
				nSelectPoint = i;
			}
		}

		if (distance > MIN_DISTANCE)
		{
			nSelectPoint = (point.x + w / 2) / w;
		}

		m_nLeftMost = 0;
		for (int i = nSelectPoint - 1; i >= 0; i--)
		{
			curve->GetPoint(i, x, y);
			if (x >= 0)
			{
				m_nLeftMost = x;
				break;
			}
		}

		m_nRightMost = 256;
		for (int i = nSelectPoint + 1; i < m_nPointCount; i++)
		{
			curve->GetPoint(i, x, y);
			if (x >= 0)
			{
				m_nRightMost = x;
				break;
			}
		}

		m_nSelectPoint = nSelectPoint;
		curve->UpdatePoint(nSelectPoint, (double)point.x / m_nWidth, (double)(m_nHeight - point.y) / m_nHeight);
		m_bCapture = true;

		Update();
	}

	void CSubCurveUI::OnMouseMove(POINT &point)
	{
		if (m_bCapture)
		{
			point.x -= (m_rcItem.right - m_rcBitmap.right) / 2;
			point.y -= (m_rcItem.bottom - m_rcBitmap.bottom) / 2;
// 			point.x = clamp((int)point.x, 0, m_nCurveWidth);
// 			point.y = clamp((int)point.y, 0, m_nCurveHeight);

			int x, y;
			int w = m_nWidth / (m_nPointCount - 1);
			int distance = MAXINT;
			int nSelectPoint = -1;
			SplineCurve* curve = m_pCurvesConfig->GetSelectSplineCurve();
			for (int i = 0; i < m_nPointCount; i++)
			{
				curve->GetPoint(i, x, y);
				if (abs(x - point.x) < distance)
				{
					distance = abs(x - point.x);
					nSelectPoint = i;
				}
			}

			if (distance > MIN_DISTANCE)
			{
				nSelectPoint = (point.x + w / 2) / w;
			}

			curve->SetPoint(m_nSelectPoint, -1.0, -1.0);

			if (point.x > m_nLeftMost && point.x < m_nRightMost)
			{
				nSelectPoint = (point.x + w / 2) / w;
				curve->GetPoint(nSelectPoint, x, y);
				if (x < 0)
				{
					m_nSelectPoint = nSelectPoint;
				}

				curve->UpdatePoint(nSelectPoint, (double)point.x / m_nWidth, (double)(m_nHeight - point.y) / m_nHeight);
			}

			Update();
		}
	}

	void CSubCurveUI::OnLButtonUp(POINT &point)
	{
		m_bCapture = false;
		m_nSelectPoint = -1;

		Update();
	}

	void CSubCurveUI::Clear(void)
	{
		if (m_pCurvesConfig)
		{
			delete m_pCurvesConfig;
			m_pCurvesConfig = 0;
		}

		if (m_hBitmap)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}

		for (int i = 0; i < PEN_SIZE; i++)
		{
			if (m_hPen[i] != NULL)
			{
				::DeleteObject(m_hPen[i]);
				m_hPen[i] = NULL;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	CCurveUI::CCurveUI(void)
	{
		m_pSubCurveUI = new CSubCurveUI();
		RECT rcSub = m_pSubCurveUI->GetPos();
		int w0 = rcSub.right - rcSub.left;
		int h0 = rcSub.bottom - rcSub.top;
		int w1 = m_rcItem.right - m_rcItem.left;
		int h1 = m_rcItem.bottom - m_rcItem.top;
		int x0 = (w1 - w0) / 2;
		int y0 = (h1 - h0) / 2;
		int x1 = x0 + rcSub.right;
		int y1 = y0 + rcSub.bottom;
		rcSub.left = x0; rcSub.top = y0;
		rcSub.right = x1; rcSub.bottom = y1;
		m_pSubCurveUI->SetFloat(true);
		m_pSubCurveUI->SetPos(rcSub);
		m_pSubCurveUI->SetManager(GetManager(), this);
		Add(m_pSubCurveUI);
	}

	CCurveUI::~CCurveUI(void)
	{
		if (m_pSubCurveUI) delete m_pSubCurveUI;
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
			return CContainerUI::GetInterface(pstrName);
		}
	}

	void CCurveUI::DoEvent(TEventUI& event)
	{

		CContainerUI::DoEvent(event);
	}

	void CCurveUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{

		CContainerUI::DoPaint(hDC, rcPaint);
	}
}
