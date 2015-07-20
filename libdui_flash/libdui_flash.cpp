// libdui_flash.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <atlcomcli.h>
#include <comutil.h>
#include <Unknwn.h>

#import "PROGID:ShockwaveFlash.ShockwaveFlash" raw_interfaces_only, named_guids, rename_namespace("e"),\
	rename("IDispatchEx", "IMyDispatchEx"), \
	rename("ICanHandleException", "IMyICanHandleException")
using namespace e;

#define DISPID_FLASHEVENT_FLASHCALL		( 0x00C5 )
#define DISPID_FLASHEVENT_FSCOMMAND		( 0x0096 )
#define DISPID_FLASHEVENT_ONPROGRESS	( 0x07A6 )

#define DECLMSG(method) \
	LRESULT method(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)

class MainFrame :
	public CWindowWnd,
	public INotifyUI,
	public _IShockwaveFlashEvents

{
public:
	MainFrame(){ };
	~MainFrame(){ }
	LPCTSTR GetWindowClassName(void) const { return _T("UIMainFrame"); };
	UINT GetClassStyle(void) const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/) { delete this; };

	void Init(void)
	{

	}

	void Notify(TNotifyUI& msg)
	{
		if (msg.sType == _T("click"))
		{
			if (msg.pSender->GetName() == _T("closebtn"))
			{
				//Close();
				PostQuitMessage(0);
				return;
			}
		}
		else if (msg.sType == _T("showactivex"))
		{
			if (msg.pSender->GetName() != _T("flash")) return;
			IShockwaveFlash* pFlash = NULL;
			CActiveXUI* pActiveX = static_cast<CActiveXUI*>(msg.pSender);
			pActiveX->GetControl(IID_IUnknown, (void**)&pFlash);

			if (pFlash != NULL)
			{
				pFlash->put_Loop(VARIANT_FALSE);
				pFlash->put_WMode(_bstr_t(_T("Transparent")));//Transparent
				pFlash->put_Movie(_bstr_t(CPaintManagerUI::GetInstancePath() + _T("\\skin\\FlashRes\\test.swf")));
				pFlash->DisableLocalSecurity();
				pFlash->put_AllowScriptAccess(_T("always"));
				BSTR response;
				pFlash->CallFunction(_T("<invoke name=\"setButtonText\" returntype=\"xml\"><arguments><string>Click me!</string></arguments></invoke>"), &response);
				pFlash->Release();
			}
		}
	}

	bool RegisterEventHandler(bool inAdvise)
	{
		IShockwaveFlash* pFlash = NULL;
		CActiveXUI* pActiveX = static_cast<CActiveXUI*>(m_pm.FindControl(_T("flash")));
		pActiveX->GetControl(IID_IUnknown, (void**)&pFlash);

		if (pFlash)
		{
			CComPtr<IConnectionPointContainer> pCPC;
			CComPtr<IConnectionPoint> pCP;

			HRESULT hr = pFlash->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
			if (FAILED(hr))
			{
				pFlash->Release();
				return false;
			}
			hr = pCPC->FindConnectionPoint(__uuidof(_IShockwaveFlashEvents), &pCP);
			if (FAILED(hr))
			{
				pFlash->Release();
				return false;
			}

			if (inAdvise)
			{
				hr = pCP->Advise((IDispatch*)this, &m_dwCookie);
			}
			else
			{
				hr = pCP->Unadvise(m_dwCookie);
			}

			pFlash->Release();
			return hr == S_OK;
		}

		return false;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(__RPC__out UINT *pctinfo)
	{
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo)
	{
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(__RPC__in REFIID riid, __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames, UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId)
	{
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
	{
		switch (dispIdMember)
		{
		case DISPID_FLASHEVENT_FLASHCALL:
		{
			if (pDispParams->cArgs != 1 || pDispParams->rgvarg[0].vt != VT_BSTR)
				return E_INVALIDARG;
			return this->FlashCall(pDispParams->rgvarg[0].bstrVal);
		}
		case DISPID_FLASHEVENT_FSCOMMAND:
		{
			if (pDispParams && pDispParams->cArgs == 2)
			{
				if (pDispParams->rgvarg[0].vt == VT_BSTR &&
					pDispParams->rgvarg[1].vt == VT_BSTR)
				{
					return OnFSCommand(pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
				}
				else
				{
					return DISP_E_TYPEMISMATCH;
				}
			}
			else
			{
				return DISP_E_BADPARAMCOUNT;
			}
		}
		case DISPID_FLASHEVENT_ONPROGRESS:
		{
			return OnProgress(*pDispParams->rgvarg[0].plVal);
		}
		case DISPID_READYSTATECHANGE:
		{
			return this->OnReadyStateChange(pDispParams->rgvarg[0].lVal);
		}
		default:
			break;
		}

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		*ppvObject = NULL;

		if (riid == IID_IUnknown)
			*ppvObject = static_cast<LPUNKNOWN>(this);
		else if (riid == IID_IDispatch)
			*ppvObject = static_cast<IDispatch*>(this);
		else if (riid == __uuidof(_IShockwaveFlashEvents))
			*ppvObject = static_cast<_IShockwaveFlashEvents*>(this);

		if (*ppvObject != NULL)	AddRef();

		return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
	}
	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		::InterlockedIncrement(&m_dwRefCount);
		return m_dwRefCount;
	}
	ULONG STDMETHODCALLTYPE Release(void)
	{
		::InterlockedDecrement(&m_dwRefCount);
		return m_dwRefCount;
	}
	HRESULT OnReadyStateChange(long lNewState)
	{
		return S_OK;
	}

	HRESULT OnProgress(int nPercent)
	{
		TCHAR szText[256] = { 0 };
		_stprintf_s(szText, _T("progress = %d\n"), nPercent);
		OutputDebugString(szText);
		return S_OK;
	}

	HRESULT OnFSCommand(_bstr_t command, _bstr_t args)
	{

		return S_OK;
	}

	HRESULT FlashCall(_bstr_t request)
	{

		return S_OK;
	}

	DECLMSG(OnKeyDown)
	{
		CDuiString str = CDuiString(_T("key = ")) + CDuiString((int)wParam) + CDuiString(_T("\n"));
		OutputDebugString(str.GetData());

		IShockwaveFlash* pFlash = NULL;
		CActiveXUI* pActiveX = static_cast<CActiveXUI*>(m_pm.FindControl(_T("flash")));
		pActiveX->GetControl(IID_IUnknown, (void**)&pFlash);

		if (pFlash)
		{
			pFlash->put_Loop(VARIANT_FALSE);
			HRESULT hr = pFlash->Stop();
			pFlash->Release();
		}

		bHandled = TRUE;
		return 0;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CControlUI* pRoot = builder.Create(_T("ui.xml"), (UINT)0, NULL, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		Init();
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::PostQuitMessage(0L);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (::IsIconic(*this)) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
				return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg) {
		case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:	   lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
		}
		if (bHandled) return lRes;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

private:
	CPaintManagerUI m_pm;
	DWORD m_dwCookie;
	DWORD m_dwRefCount;
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\FlashRes"));

	MainFrame* pFrame = new MainFrame;
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, NULL, UI_WNDSTYLE_DIALOG, 0);
	pFrame->CenterWindow();
	pFrame->ShowWindow(true);
	CPaintManagerUI::MessageLoop();
	delete pFrame;

	::CoUninitialize();
	return 0;
}

