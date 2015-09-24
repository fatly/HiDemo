#pragma once

namespace DuiLib
{
#define MAX_PEN_SIZE 10
	class e::CurvesConfig;
	class CCurveUI : public CControlUI
	{
	public:
		CCurveUI(void);
		~CCurveUI(void);
		LPCTSTR GetClass(void) const;
		LPVOID GetInterface(LPCTSTR pstrName);
		bool Init(int nWidth, int nHeight);
		virtual void DoPaint(HDC hDC, const RECT& rcPaint) override;
		virtual void DoEvent(TEventUI& event) override;
	protected:
		bool InitPen(int nCount);
		void Update(void);
		void Clear(void);
	protected:
		RECT	m_rcBitmap;
		RECT	m_rcCorners;
		HBITMAP m_hBitmap;
		e::CurvesConfig* m_pCurves;
		CComboUI m_ctrlSelected;
		HPEN m_hPen[MAX_PEN_SIZE];
	};
}
