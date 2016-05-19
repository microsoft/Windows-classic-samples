// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __APP_H__
#define __APP_H__

#include "stdafx.h"
#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CAppSub
Summary: Application Event Subscriber
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CAppSub : 
	public CSysLCESub,
	public IComAppEvents
{
public:
	CAppSub()
	{
		m_pSpy = NULL;
	}
	CAppSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	DECLARE_NOT_AGGREGATABLE(CAppSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CAppSub)
		COM_INTERFACE_ENTRY(IComAppEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()


    virtual EventEnum EventType() { return Application; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComAppEvents; }

	STDMETHODIMP OnAppActivation(COMSVCSEVENTINFO * pInfo, GUID guidApp)
	{
		if (m_pSpy)
		{
			CComBSTR bstrGuidApp = GuidToBstr(guidApp);
			m_pSpy->AddEventToList(pInfo->perfCount, L"OnAppActivation", bstrGuidApp);	
			m_pSpy->AddParamValueToList(L"guidApp", bstrGuidApp);

			IF_AUDIT_DO(OnAppActivation)(pInfo->perfCount, bstrGuidApp);
		}
		return S_OK;
	}

	STDMETHODIMP OnAppShutdown(COMSVCSEVENTINFO * pInfo, GUID guidApp)
	{
		if (m_pSpy)
		{
			CComBSTR bstrGuidApp = GuidToBstr(guidApp);
			m_pSpy->AddEventToList(pInfo->perfCount, L"OnAppShutdown", bstrGuidApp);	
			m_pSpy->AddParamValueToList(L"guidApp", bstrGuidApp);
			IF_AUDIT_DO(OnAppShutdown)(pInfo->perfCount, bstrGuidApp);
		}

		return S_OK;
	}

	STDMETHODIMP OnAppForceShutdown(COMSVCSEVENTINFO * pInfo, GUID guidApp)
	{
		if (m_pSpy)
		{
			CComBSTR bstrGuidApp = GuidToBstr(guidApp);
			m_pSpy->AddEventToList(pInfo->perfCount, L"OnAppForceShutdown", bstrGuidApp);	
			m_pSpy->AddParamValueToList(L"guidApp", bstrGuidApp);
//			IF_AUDIT_DO(OnAppShutdown)(pInfo->perfCount, bstrGuidApp);
		}

	return S_OK;
	}
};

#endif //__APP_H__
