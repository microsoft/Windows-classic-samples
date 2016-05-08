// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __IDENTITYSUB_H__
#define __IDENTITYSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CIdentitySub
Summary: Identity Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CIdentitySub : 
	public CSysLCESub,
	public IComIdentityEvents
{
public:
	CIdentitySub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CIdentitySub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CIdentitySub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CIdentitySub)
		COM_INTERFACE_ENTRY(IComIdentityEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Identity; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComIdentityEvents; }

	STDMETHODIMP OnIISRequestInfo(
							 COMSVCSEVENTINFO * pInfo,
							 ULONG64 ObjId,
							 LPCWSTR pwszClientIP,
							 LPCWSTR pwszServerIP,
							 LPCWSTR pwszURL
							)
	{	
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnIISRequestInfo", GuidToBstr(pInfo->guidApp));

		WCHAR szObjectID[32];
		wsprintfW(szObjectID, L"%#016I64X", ObjId);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

		m_pSpy->AddParamValueToList(L"ClientIP", pwszClientIP);
		m_pSpy->AddParamValueToList(L"ServerIP", pwszServerIP);
		m_pSpy->AddParamValueToList(L"URL", pwszURL);


		IF_AUDIT_DO(OnIISRequestInfo)(pInfo->perfCount, GuidToBstr(pInfo->guidApp),							
									szObjectID,						
									pwszClientIP,						
									pwszServerIP,
									pwszURL
									);

		return S_OK;
	}

};

#endif //__IDENTITYSUB_H__
