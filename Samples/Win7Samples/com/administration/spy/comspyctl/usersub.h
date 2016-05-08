// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __USERSUB_H__
#define __USERSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CUserSub
Summary: User Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CUserSub : 
	public CSysLCESub,
	public IComUserEvent
{
public:

	CUserSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	CUserSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CUserSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CUserSub)
		COM_INTERFACE_ENTRY(IComUserEvent)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return User; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComUserEvent; }

	STDMETHODIMP OnUserEvent(COMSVCSEVENTINFO * pInfo, VARIANT * var)
	{
		if (var->vt == VT_BSTR)
		{
			m_pSpy->AddEventToList(pInfo->perfCount, L"OnUserEvent", GuidToBstr(pInfo->guidApp));
			m_pSpy->AddParamValueToList(L"String Value",  var->bstrVal);	
		}
		else if (var->vt == VT_UNKNOWN)
		{
			CComPtr<IMtsEventInfo> spEvtInfo;
			
			var->punkVal->QueryInterface(IID_PPV_ARGS(&spEvtInfo));
			if(spEvtInfo)
			{
				CComBSTR bstrTemp;
				spEvtInfo->get_DisplayName(&bstrTemp.m_str);
				m_pSpy->AddEventToList(pInfo->perfCount, bstrTemp.m_str, GuidToBstr(pInfo->guidApp));	

				long lCount;
				spEvtInfo->get_Count(&lCount);
				if (lCount)
				{
					VARIANT varValue;
					LPWSTR* pKeys = new LPWSTR[lCount+1];
					CComPtr<IUnknown> spUnk;
					spEvtInfo->get_Names(&spUnk);
					_ASSERTE(spUnk);
					CComPtr<IEnumString> spEnum;
					spUnk->QueryInterface(IID_PPV_ARGS(&spEnum));
					_ASSERTE(spEnum);
					spUnk.Release();
					ULONG lFetched;
					spEnum->Next(lCount, pKeys, &lFetched);
					spEnum.Release();				
					_ASSERTE(lFetched == lCount);
					UINT i;
					for (i=0; i<(UINT)lFetched; i++)
					{
						bstrTemp = pKeys[i];
						spEvtInfo->get_Value(bstrTemp, &varValue);
						if (varValue.vt == VT_BSTR)
						{
							m_pSpy->AddParamValueToList(pKeys[i],  varValue.bstrVal);	
							VariantClear(&varValue);
						}
					}
					for (i=0;i<(UINT)lCount;i++)
						CoTaskMemFree(pKeys[i]);

					delete [] pKeys;
				}
				spEvtInfo.Release();
			}
		}
		else
			m_pSpy->AddEventToList(pInfo->perfCount, L"OnUserEvent", GuidToBstr(pInfo->guidApp));


		return S_OK;
	}

};

#endif //__USERSUB_H__
