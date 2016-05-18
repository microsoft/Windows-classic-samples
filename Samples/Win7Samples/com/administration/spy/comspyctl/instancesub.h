// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __INSTANCESUB_H__
#define __INSTANCESUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CInstanceSub
Summary: Instance Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CInstanceSub : 
	public CSysLCESub,
	public IComInstanceEvents
{
public:
	CInstanceSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CInstanceSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CInstanceSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CInstanceSub)
		COM_INTERFACE_ENTRY(IComInstanceEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Instance; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComInstanceEvents; }

	STDMETHODIMP OnObjectCreate(COMSVCSEVENTINFO * pInfo, REFGUID guidActivity, REFCLSID clsid, REFGUID tsid, ULONG64 CtxtID, ULONG64 ObjectID)
	{

		m_pSpy->AddEventToList(pInfo->perfCount, L"OnObjectCreate", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);
		CComBSTR bstrClsid = GuidToBstr(clsid);
		CComBSTR bstrTsid = GuidToBstr(tsid);

		m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);
		m_pSpy->AddParamValueToList(L"clsid", bstrClsid);
		m_pSpy->AddParamValueToList(L"tsid", bstrTsid);

		WCHAR szCtxtID[32];
        wsprintfW(szCtxtID,L"%#016I64X", CtxtID);
		m_pSpy->AddParamValueToList(L"CtxtID", szCtxtID);

		WCHAR szObjectID[32];
        wsprintfW(szObjectID,L"%#016I64X", ObjectID);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

		

		IF_AUDIT_DO(OnObjectCreate)(pInfo->perfCount, GuidToBstr(pInfo->guidApp),							
									bstrGuidActivity,						
									bstrClsid,						
									bstrTsid,
									szCtxtID,
									szObjectID);

		return S_OK;
	}

	STDMETHODIMP OnObjectDestroy(COMSVCSEVENTINFO * pInfo, ULONG64 CtxtID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnObjectDestroy",  GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32];
		wsprintfW(szCtxtID,L"%#016I64X", CtxtID);
		m_pSpy->AddParamValueToList(L"CtxtID", szCtxtID);

		IF_AUDIT_DO(OnObjectDestroy)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),														
									 szCtxtID);

		return S_OK;
	}

};

#endif //__INSTANCESUB_H__
