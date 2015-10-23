#include "stdafx.h"
#include "UICurve.h"
#include "CurvesConfig.h"

#define  MIN_DISTANCE 12

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
		CreatePens(8);
		SetSize(256, 256);
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
		m_nLineWidth = 1;
		memset(&m_rcBitmap, 0, sizeof(m_rcBitmap));
		memset(&m_rcCorners, 0, sizeof(m_rcCorners));
		memset(&m_hPen, 0, sizeof(HPEN) * PEN_SIZE);
		CreatePens(8);
		SetSize(nWidth, nHeight);
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

	void CSubCurveUI::SetLineWidth(int nLineWidth)
	{
		m_nLineWidth = nLineWidth;
	}

	bool CSubCurveUI::SetSize(int nWidth, int nHeight)
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

		if (m_hBitmap != NULL)	::DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;

		m_rcBitmap.left = 0;
		m_rcBitmap.top = 0;
		m_rcBitmap.right = nWidth;
		m_rcBitmap.bottom = nHeight;

		m_nWidth = nWidth;
		m_nHeight = nHeight;
		
		SetFixedWidth(nWidth);
		SetFixedHeight(nHeight);
		return true;
	}

	bool CSubCurveUI::CreatePens(int nCount)
	{
		limit(nCount, 6, PEN_SIZE);

		COLORREF clrPenColors[] = {
			RGB(45, 45, 45),	//C
			RGB(255, 0, 0),		//R
			RGB(0, 255, 0),		//B
			RGB(0, 0, 255),		//G
			RGB(255,255,0),		//A
			RGB(210, 210, 210),	//can kao
			RGB(240, 170, 30),	//edge
			RGB(10, 10, 10)		//point
		};

		const int w = m_nLineWidth;
		const int nPenWidths[] = { w, w, w, w, w, 1, 1, 4, w, w, w, w };
		for (int i = 0; i <nCount; i++)
		{
			m_hPen[i] = ::CreatePen(PS_SOLID, nPenWidths[i], clrPenColors[i]);
			assert(m_hPen[i] != NULL);
			if (m_hPen[i] == NULL) return false;
		}

		return true;
	}

	void CSubCurveUI::DoInit(void)
	{
		Update();
		__super::DoInit();
	}

	void CSubCurveUI::DoEvent(TEventUI& event)
	{
		TCHAR szText[64] = { 0 };
//		_stprintf_s(szText, _T("CSubCurveUI::DoEvent()->t=%d,x=%d,y=%d\n"), event.Type, event.ptMouse.x, event.ptMouse.y);
//		OutputDebugString(szText);

		if (event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			POINT point = event.ptMouse;
			point.x = CLAMP(point.x - GetX(), 0, m_nWidth);
			point.y = CLAMP(point.y - GetY(), 0, m_nHeight);
			switch (event.Type)
			{
			case UIEVENT_BUTTONDOWN:
				OnLButtonDown(point);
				break;
			case UIEVENT_MOUSEMOVE:
				OnMouseMove(point);
				break;
			case UIEVENT_BUTTONUP:
				OnLButtonUp(point);
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
			CRenderEngine::DrawColor(hDC, m_rcItem, 0xFFFFFF00);
			CRenderEngine::DrawImage(hDC, m_hBitmap, m_rcItem, rcPaint, m_rcBitmap, m_rcCorners, false, 255);
		}
		else
		{
			CControlUI::DoPaint(hDC, rcPaint);
		}
	}

	void CSubCurveUI::Update(void)
	{
		assert(m_hBitmap);
		HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		HDC  hMemDC = ::CreateCompatibleDC(NULL);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, m_hBitmap);
		::FillRect(hMemDC, &m_rcBitmap, hBrush);

		int w = m_nWidth;
		int h = m_nHeight;
		int nChannel = m_pCurvesConfig->GetSelectIndex();

		//画边框
		{	
			int x0 = m_rcBitmap.left;
			int y0 = m_rcBitmap.top;
			int x1 = m_rcBitmap.right - 1;
			int y1 = m_rcBitmap.bottom - 1;
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[6]);
			::MoveToEx(hMemDC, x0, y0, NULL);
			::LineTo(hMemDC, x1, y0);
			::LineTo(hMemDC, x1, y1);
			::LineTo(hMemDC, x0, y1);
			::LineTo(hMemDC, x0, y0);
			::SelectObject(hMemDC, hOldPen);
		}

		//画没选中的B线
		{	
			for (int c = CURVE_CHANNEL_A; c >= CURVE_CHANNEL_C; c--)
			{
				HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[c]);
				SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(c);
				if (!curve->IsIdentity() || c == nChannel) continue;

				int y = h - curve->GetSample(0, (double)(h-1));
				if (y >= h) y -= 1;
				::MoveToEx(hMemDC, 0, y, NULL);

				for (int x = 1; x < curve->GetSampleCount() && x < w; x++)
				{
					y = h - curve->GetSample(x, (double)(h - 1));
					if (y >= h) y -= 1;
					::LineTo(hMemDC, x, y);
				}
				::SelectObject(hMemDC, hOldPen);
			}
		}

		//画参考线
		{	
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[5]);
			SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(5);

			int y = h - curve->GetSample(0, (double)(h - 1));
			if (y >= h) y -= 1;
			::MoveToEx(hMemDC, 0, y, NULL);

			for (int x = 1; x < curve->GetSampleCount() && x < w; x++)
			{
				y = h - curve->GetSample(x, (double)(h - 1));
				if (y >= h) y -= 1;
				::LineTo(hMemDC, x, y);
			}
			::SelectObject(hMemDC, hOldPen);
		}

		//画选中的B线
		{	
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[nChannel]);
			SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(nChannel);

			int y = h - curve->GetSample(0, (double)(h - 1));
			if (y >= h) y -= 1;
			::MoveToEx(hMemDC, 0, y, NULL);

			for (int x = 1; x < curve->GetSampleCount() && x < w; x++)
			{
				y = h - curve->GetSample(x, (double)(h - 1));
				if (y >= h) y -= 1;
				::LineTo(hMemDC, x, y);
			}
			::SelectObject(hMemDC, hOldPen);
		}

		//画点
		{	
			HPEN hOldPen = (HPEN)::SelectObject(hMemDC, m_hPen[7]);
			SplineCurve* curve = m_pCurvesConfig->GetSplineCurve(nChannel);
			for (int i = 0; i < curve->GetPointCount(); i++)
			{
				int x = 0, y = 0;
				curve->GetPoint(i, x, y, (double)(h - 1));
				y = h - y;
				if (y >= h) y -= 1;

				::MoveToEx(hMemDC, x, y, NULL);
				::LineTo(hMemDC, x, y);
			}
			::SelectObject(hMemDC, hOldPen);
		}
	
		::SelectObject(hMemDC, hOldBitmap);
		::DeleteObject(hBrush);
		::DeleteDC(hMemDC);

		NeedUpdate();
	}

	void CSubCurveUI::OnLButtonDown(POINT &point)
	{
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

		m_nRightMost = m_nWidth;
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
 			point.x = clamp((int)point.x, 0, m_nWidth);
 			point.y = clamp((int)point.y, 0, m_nHeight);

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
		//m_pSubCurveUI = new CSubCurveUI();
		//assert(m_pSubCurveUI);
	}

	CCurveUI::~CCurveUI(void)
	{
		//if (m_pSubCurveUI) delete m_pSubCurveUI;
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
			return __super::GetInterface(pstrName);
		}
	}

	void CCurveUI::DoInit(void)
	{
		//curve preset
		RECT pos = { 0 };
		int x = 10, y = 5, w = 200, h = 23;
		auto cbUI = new CComboUI();
		Add(cbUI);
		pos = RECT{ x, y, x + w, y + h };
		cbUI->SetName(_T("ctrl_curve_preset"));
		cbUI->SetBkColor(RGB(30, 170, 240) | 0xff000000);
		cbUI->SetFloat(true);
		cbUI->SetPos(pos);

		//SubCurveUI
		x = 10, y = 33, w = 200, h = 200;
		auto uiSub = new CSubCurveUI(w, h);
		Add(uiSub);
		pos = RECT{ x, y, x + w, y + h };
		uiSub->SetName(_T("ctrl_sub_curve"));
		uiSub->SetFloat(true);
		uiSub->SetPos(pos);
		//channel select
		x = 10, y = 238, w = 120, h = 23;
		cbUI = new CComboUI();
		Add(cbUI);
		pos = RECT{ x, y, x + w, y + h };
		cbUI->SetName(_T("ctrl_curve_selchannel"));
		cbUI->SetBkColor(RGB(30, 170, 240) | 0xff000000);
		cbUI->SetFloat(true);
		cbUI->SetPos(pos);
		TCHAR name[64] = {0};
		TCHAR* text[] = { _T("Color"), _T("Red"), _T("Green"), _T("Blue"), _T("Alpha") };
		for (int i = 0; i < 5; i++)
		{
			_stprintf_s(name, _T("ctrl_curve_channle_option_%d"), i);
			auto op = new CListLabelElementUI();
			op->SetName(name);
			op->SetText(text[i]);
			cbUI->Add(op);
			if (i == 0) op->Select(true);
		}

		//curves reset
		x = 140, y = 238, w = 70, h = 23;
		auto btn = new CButtonUI();
		Add(btn);
		pos = RECT{ x, y, x + w, y + h };
		btn->SetName(_T("ctrl_curve_reset"));
		btn->SetText(_T("默认"));
		btn->SetBkColor(RGB(30, 170, 240) | 0xff000000);
		btn->SetFloat(true);
		btn->SetPos(pos);

		__super::DoInit();
	}

	void CCurveUI::DoEvent(TEventUI& event)
	{
// 		TCHAR szText[64] = { 0 };
// 		_stprintf_s(szText, _T("CCurveUI::DoEvent()->%d\n"), event.Type);
// 		OutputDebugString(szText);

		__super::DoEvent(event);
	}

	void CCurveUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{

		__super::DoPaint(hDC, rcPaint);
	}
}
