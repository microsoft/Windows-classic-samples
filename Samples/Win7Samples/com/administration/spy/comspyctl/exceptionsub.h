// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __EXCEPTIONSUB_H__
#define __EXCEPTIONSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CExceptionSub
Summary: Exception Event Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CExceptionSub : 
	public CSysLCESub,
	public IComExceptionEvents
{
public:
	CExceptionSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CExceptionSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CExceptionSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CExceptionSub)
		COM_INTERFACE_ENTRY(IComExceptionEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Exception; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComExceptionEvents; }

	STDMETHODIMP OnExceptionUser(
							 COMSVCSEVENTINFO * pInfo,
							 ULONG code,
							 ULONG64 addr,
							 LPCWSTR pwszStackTrace
							)
	{	
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnExceptionUser", GuidToBstr(pInfo->guidApp));

		WCHAR szCode[16];
        wsprintfW(szCode, L"%#08X", code);
		m_pSpy->AddParamValueToList(L"code", szCode);

		WCHAR szAddress[32];
		wsprintfW(szAddress, L"%#016I64X", addr);
		m_pSpy->AddParamValueToList(L"Address", szAddress);

		m_pSpy->AddParamValueToList(L"StackTrace", pwszStackTrace);
		return S_OK;


		IF_AUDIT_DO(OnExceptionUser)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),														
									  szCode,
									  szAddress,
									  pwszStackTrace
									  );	
	}

};

#endif //__EXCEPTIONSUB_H__
