#include "DShowHelp.h"
#include <uuids.h>
#include <tchar.h>

namespace e
{
	HRESULT GetMonikerByCLSID(_In_   REFCLSID rclsid, TCHAR* deviceName, IMoniker** pMoniker)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		*pMoniker = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);

		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		IEnumMoniker *pEm = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
		if (!SUCCEEDED(hr))
		{
			pCreateDevEnum->Release();
			return hr;
		}

		pEm->Reset();
		IMoniker *pM;
		while ((hr = pEm->Next(1, &pM, NULL)) == S_OK && *pMoniker == NULL)
		{
			IPropertyBag *pBag = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(_T("CLSID"), &var, NULL);
				if (SUCCEEDED(hr))
				{
					CLSID clsid = { 0 };
					hr = CLSIDFromString(var.bstrVal, &clsid);
					if (SUCCEEDED(hr))
					{
						VARIANT var1;
						var1.vt = VT_BSTR;
						hr = pBag->Read(TEXT("FriendlyName"), &var1, NULL);

						if (SUCCEEDED(hr))
						{
							if (IsEqualCLSID(rclsid, clsid) && (_tcscmp(var1.bstrVal, deviceName) == 0))
							{
								*pMoniker = pM;
								pM->AddRef();
							}

							::SysFreeString(var1.bstrVal);
						}
					}

					::SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pM->Release();
		}
		pEm->Release();

		if (*pMoniker == NULL)
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT GetMonikerByName(REFCLSID deviceCatogory, TCHAR* deviceName, IBaseFilter** pFilter)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		IMoniker* pMoniker = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);

		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		IEnumMoniker *pEm = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(deviceCatogory, &pEm, 0);
		if (S_OK != hr)
		{
			pCreateDevEnum->Release();
			return hr;
		}

		pEm->Reset();
		IMoniker *pM;
		while ((pEm->Next(1, &pM, NULL)) == S_OK && pMoniker == NULL)
		{
			IPropertyBag *pBag = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{

				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(TEXT("FriendlyName"), &var, NULL);

				if (SUCCEEDED(hr))
				{
					int nLen1 = _tcslen(var.bstrVal);
					int nLen2 = _tcslen(deviceName);

					if (nLen1 > nLen2)
					{
						nLen1 = nLen2;
					}

					if ((_tcsncmp(var.bstrVal, deviceName, nLen1) == 0))
					{
						pMoniker = pM;
						hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
						pM->AddRef();
					}

					::SysFreeString(var.bstrVal);
				}

				pBag->Release();
			}
			pM->Release();
		}

		pEm->Release();

		if (pMoniker == NULL)
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT GetMonikerByDisplayName(REFCLSID deviceCatogory, TCHAR* displayName, IBaseFilter** pFilter)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		IMoniker* pMoniker = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);

		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		IEnumMoniker *pEm = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(deviceCatogory, &pEm, 0);
		if (S_OK != hr)
		{
			pCreateDevEnum->Release();
			return hr;
		}

		pEm->Reset();
		IMoniker *pM;
		while ((pEm->Next(1, &pM, NULL)) == S_OK && pMoniker == NULL)
		{
			IPropertyBag *pBag = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr))
			{
// 				VARIANT var;
// 				var.vt = VT_BSTR;
// 				pBag->Read(TEXT("FriendlyName"), &var, NULL);

				LPOLESTR name;
				hr = pM->GetDisplayName(0, 0, &name);

				if (SUCCEEDED(hr))
				{
					int nLen1 = _tcslen(name);
					int nLen2 = _tcslen(displayName);

					if (nLen1 > nLen2)
					{
						nLen1 = nLen2;
					}

					if ((_tcsncmp(name, displayName, nLen1) == 0))
					{
						pMoniker = pM;
						hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
						pM->AddRef();
					}
				}

				pBag->Release();
			}

			pM->Release();
		}

		pEm->Release();

		if (pMoniker == NULL)
		{
			hr = E_FAIL;
		}

		return hr;
	}


	HRESULT GetPinCountForOneType(IUnknown* pFilter, PIN_DIRECTION wantDir, int* pPinCount)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		(*pPinCount) = 0;

		hr = pFilter->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							(*pPinCount)++;
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		return hr;
	}

	HRESULT GetAMVideoConfigForMorePin(IUnknown* pUnk, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							IKsPropertySet* pPS;
							hr = pPin->QueryInterface(IID_IKsPropertySet, (VOID**)&pPS);
							if (SUCCEEDED(hr))
							{
								GUID guid = { 0 };
								DWORD dwReturn = 0;
								hr = pPS->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, 0, 0, &guid, sizeof(guid), &dwReturn);

								if (SUCCEEDED(hr) && ::IsEqualGUID(guid, PIN_CATEGORY_CAPTURE))
								{
									hr = pPin->QueryInterface(IID_IAMStreamConfig, (VOID**)ppConfig);
									if (SUCCEEDED(hr))
									{
										bFound = TRUE;
									}
								}

								pPS->Release();
							}
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		return hr;
	}

	HRESULT GetAMVideoConfigForOnePin(IUnknown* pFilter, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;

		hr = pFilter->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEnumerator = NULL;
			hr = pBF->EnumPins(&pEnumerator);

			if (SUCCEEDED(hr))
			{
				pEnumerator->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					while (((pEnumerator->Next(1, &pPin, NULL)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{

							hr = pPin->QueryInterface(IID_IAMStreamConfig, (VOID**)ppConfig);
							if (SUCCEEDED(hr))
							{
								bFound = TRUE;
							}
						}

						pPin->Release();
					}
				}

				pEnumerator->Release();
			}

			pBF->Release();
		}

		return hr;
	}


	HRESULT GetAMVideoConfig(IUnknown* pFilter, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig)
	{
		int pinCount = 0;
		HRESULT hr = S_OK;
		hr = GetPinCountForOneType(pFilter, wantDir, &pinCount);

		if (SUCCEEDED(hr) && pinCount > 0)
		{
			if (pinCount > 1)
			{
				hr = GetAMVideoConfigForMorePin(pFilter, wantDir, ppConfig);
			}
			else
			{
				hr = GetAMVideoConfigForOnePin(pFilter, wantDir, ppConfig);
			}
		}
		else
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT GetCameraConfig(IUnknown* pFilter, PIN_DIRECTION wantDir, IAMStreamConfig** ppConfig)
	{
		HRESULT hr = S_OK;

		IPin* pCamPin = NULL;
		//GetCameraPin(pFilter,wantDir,&pCamPin);
		GetPinByDirection(pFilter, wantDir, &pCamPin);

		pCamPin->QueryInterface(IID_IAMStreamConfig, (VOID**)ppConfig);

		pCamPin->Release();

		return hr;
	}


	HRESULT GetCameraPin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		*ppPin = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							//IKsPropertySet* pPS;
							//hr = pPin->QueryInterface(IID_IKsPropertySet,(VOID**)&pPS);
							//if(SUCCEEDED(hr))
							//{													
							//	GUID guid = { 0 };
							//	DWORD dwReturn = 0;
							//	hr = pPS->Get(AMPROPSETID_Pin,AMPROPERTY_PIN_CATEGORY,0,0,&guid,sizeof(guid),&dwReturn);
							//
							//	if(SUCCEEDED(hr) && ::IsEqualGUID(guid,PIN_CATEGORY_CAPTURE))
							//	{									
							//			if(SUCCEEDED(hr))
							//			{

							//if (bFound)
							{
								*ppPin = pPin;
								(*ppPin)->AddRef();

							}
							bFound = TRUE;
							//			}								
							//	}
							//	
							//	pPS->Release();
							//}
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		if (*ppPin == NULL)
		{
			hr = E_NOTIMPL;
		}

		return hr;
	}

	HRESULT GetPinByDirectionForMorePin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		*ppPin = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							IKsPropertySet* pPS;
							hr = pPin->QueryInterface(IID_IKsPropertySet, (VOID**)&pPS);
							if (SUCCEEDED(hr))
							{
								GUID guid = { 0 };
								DWORD dwReturn = 0;
								hr = pPS->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, 0, 0, &guid, sizeof(guid), &dwReturn);

								if (SUCCEEDED(hr) && ::IsEqualGUID(guid, PIN_CATEGORY_CAPTURE))
								{
									if (SUCCEEDED(hr))
									{
										*ppPin = pPin;
										(*ppPin)->AddRef();
										bFound = TRUE;
									}
								}

								pPS->Release();
							}
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		if (*ppPin == NULL)
		{
			hr = E_NOTIMPL;
		}

		return hr;
	}
	HRESULT GetPinByDirectionForOnePin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		*ppPin = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							*ppPin = pPin;
							(*ppPin)->AddRef();
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		if (*ppPin == NULL)
		{
			hr = E_NOTIMPL;
		}

		return hr;
	}

	HRESULT GetPinByDirection(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin)
	{
		int pinCount = 0;
		HRESULT hr = S_OK;
		hr = GetPinCountForOneType(pUnk, wantDir, &pinCount);

		if (SUCCEEDED(hr) && pinCount > 0)
		{
			if (pinCount > 1)
			{
				hr = GetPinByDirectionForMorePin(pUnk, wantDir, ppPin);
			}
			else
			{
				hr = GetPinByDirectionForOnePin(pUnk, wantDir, ppPin);
			}
		}
		else
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT GetCrossbarPin(IUnknown* pUnk, PIN_DIRECTION wantDir, LPTSTR name, IPin** ppPin)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		*ppPin = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_INFO pininfo;
						hr = pPin->QueryPinInfo(&pininfo);

						int nLen1 = _tcslen(pininfo.achName);
						int nLen2 = _tcslen(name);

						if (nLen1 > nLen2)
						{
							nLen1 = nLen2;
						}

						if (SUCCEEDED(hr) && (pininfo.dir == wantDir) && _tcsncmp(pininfo.achName, name, nLen1) == 0)
						{
							*ppPin = pPin;
							(*ppPin)->AddRef();
							bFound = TRUE;
						}

						pPin->Release();
					}
				}

				pEP->Release();
			}

			pBF->Release();
		}

		if (*ppPin == NULL)
		{
			hr = E_NOTIMPL;
		}

		return hr;
	}

	HRESULT GetMonikerByDevName(REFCLSID deviceCatogory, TCHAR* deviceName, IMoniker** pMoniker)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		*pMoniker = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);

		if (!SUCCEEDED(hr))
		{
			return hr;
		}

		IEnumMoniker *pEm = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(deviceCatogory, &pEm, 0);
		if (!SUCCEEDED(hr))
		{
			pCreateDevEnum->Release();
			return hr;
		}

		pEm->Reset();
		IMoniker *pM;
		while ((hr = pEm->Next(1, &pM, NULL)) == S_OK && *pMoniker == NULL)
		{
			IPropertyBag *pBag = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);

			if (SUCCEEDED(hr))
			{
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(TEXT("FriendlyName"), &var, NULL);

				if (SUCCEEDED(hr))
				{
					int nLen1 = _tcslen(var.bstrVal);
					int nLen2 = _tcslen(deviceName);

					if (nLen1 > nLen2)
					{
						nLen1 = nLen2;
					}

					if ((wcsncmp(var.bstrVal, deviceName, nLen1) == 0))
					{
						*pMoniker = pM;
						pM->AddRef();
					}

					::SysFreeString(var.bstrVal);
				}

				pBag->Release();
			}
			pM->Release();
		}
		pEm->Release();

		if (*pMoniker == NULL)
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT GetMediaPin(IUnknown* pUnk, PIN_DIRECTION wantDir, IPin** ppPin, bool bVideo)
	{
		HRESULT hr = S_OK;
		IBaseFilter* pBF = NULL;
		*ppPin = NULL;

		hr = pUnk->QueryInterface(IID_IBaseFilter, (VOID**)&pBF);
		if (SUCCEEDED(hr))
		{
			IEnumPins* pEP = NULL;
			hr = pBF->EnumPins(&pEP);

			if (SUCCEEDED(hr))
			{
				pEP->Reset();
				if (SUCCEEDED(hr))
				{
					IPin* pPin = NULL;
					BOOL bFound = FALSE;
					DWORD dwFetched = 0;
					while (((pEP->Next(1, &pPin, &dwFetched)) == S_OK) && !bFound)
					{
						PIN_DIRECTION fetchedDir;
						hr = pPin->QueryDirection(&fetchedDir);

						if (SUCCEEDED(hr) && (fetchedDir == wantDir))
						{
							AM_MEDIA_TYPE *mt = NULL;
							IEnumMediaTypes *pMediaType = NULL;
							hr = pPin->EnumMediaTypes(&pMediaType);
							DWORD dw = 0;
							while (pMediaType->Next(1, &mt, &dw) == S_OK)
							{
								if (bVideo)
								{
									if (MEDIATYPE_Video == mt->majortype)
									{
										*ppPin = pPin;
										(*ppPin)->AddRef();
										bFound = TRUE;
									}
								}
								else
								{
									if (MEDIATYPE_Audio == mt->majortype)
									{
										*ppPin = pPin;
										(*ppPin)->AddRef();
										bFound = TRUE;
									}
								}
							}
							pMediaType->Release();
							pPin->Release();
						}
					}

					pEP->Release();
				}

				pBF->Release();
			}
		}

		if (*ppPin == NULL)
		{
			hr = E_NOTIMPL;
		}

		return hr;
	}

	HRESULT GetFirstDev(REFCLSID deviceCatogory, IBaseFilter** pFilter, bool bFirst)
	{
		ICreateDevEnum *pCreateDevEnum = 0;
		IMoniker* pMoniker = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum
			, NULL
			, CLSCTX_INPROC_SERVER
			, IID_ICreateDevEnum
			, (void**)&pCreateDevEnum);

		if (S_OK != hr)
		{
			return hr;
		}

		IEnumMoniker *pEm = 0;
		hr = pCreateDevEnum->CreateClassEnumerator(deviceCatogory, &pEm, 0);
		if (S_OK != hr)
		{
			pCreateDevEnum->Release();
			return hr;
		}
		pEm->Reset();

		bool bFound = false;
		int n = 0;
		IMoniker *pM;
		while ((pEm->Next(1, &pM, NULL)) == S_OK  && !bFound)
		{
			hr = S_FALSE;
			if (bFirst)
			{
				hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				pM->AddRef();
				bFound = true;
			}
			else
			{
				if (n > 0)
				{
					hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
					pM->AddRef();
					bFound = true;
				}

				n++;
			}
		}
		pM->Release();

		pEm->Release();

		return hr;
	}
}
