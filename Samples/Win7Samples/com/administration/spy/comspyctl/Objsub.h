// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __ObjSub_H__
#define __ObjSub_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CObjPoolSub
Summary: Object Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CObjSub : 
	public CSysLCESub,
	public IComObjectEvents
{
public:
	CObjSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CObjSub()
	{
		m_pSpy = NULL;
	}


	DECLARE_NOT_AGGREGATABLE(CObjSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CObjSub)
		COM_INTERFACE_ENTRY(IComObjectEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Object; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComObjectEvents; }

	STDMETHODIMP OnDisableCommit(COMSVCSEVENTINFO * pInfo, ULONG64 ContextID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnDisableCommit", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32];
		wsprintfW(szCtxtID,  L"%#016I64X", ContextID);	
		m_pSpy->AddParamValueToList(L"ContextID", szCtxtID);

		IF_AUDIT_DO(OnSetComplete)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), szCtxtID);
		return S_OK;
	}

	STDMETHODIMP  OnEnableCommit(COMSVCSEVENTINFO * pInfo, ULONG64 ContextID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnEnableCommit", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32];
		wsprintfW(szCtxtID,  L"%#016I64X", ContextID);	
		m_pSpy->AddParamValueToList(L"ContextID", szCtxtID);

		IF_AUDIT_DO(OnSetComplete)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), szCtxtID);
		return S_OK;
	}

	STDMETHODIMP  OnSetComplete(COMSVCSEVENTINFO * pInfo, ULONG64 ContextID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnSetComplete", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32];
		wsprintfW(szCtxtID,  L"%#016I64X", ContextID);	
		m_pSpy->AddParamValueToList(L"ContextID", szCtxtID);

		IF_AUDIT_DO(OnSetComplete)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), szCtxtID);
		return S_OK;
	}

	STDMETHODIMP  OnSetAbort(COMSVCSEVENTINFO * pInfo, ULONG64 ContextID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnSetAbort", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32];
		wsprintfW(szCtxtID,  L"%#016I64X", ContextID);	
		m_pSpy->AddParamValueToList(L"ContextID", szCtxtID);

		IF_AUDIT_DO(OnSetComplete)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), szCtxtID);
		return S_OK;
	}

	STDMETHODIMP  OnObjectDeactivate(COMSVCSEVENTINFO * pInfo, ULONG64 CtxtID, ULONG64 ObjectID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnObjectDeactivate", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32], szObjID[32];

		wsprintfW(szCtxtID,L"%#016I64X", CtxtID);	
		m_pSpy->AddParamValueToList(L"CtxtID", szCtxtID);
		wsprintfW(szObjID,L"%#016I64X", ObjectID);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjID);

		IF_AUDIT_DO(OnObjectActivate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szCtxtID, szObjID);
		return S_OK;
	}

	STDMETHODIMP  OnObjectActivate(COMSVCSEVENTINFO * pInfo, ULONG64 CtxtID, ULONG64 ObjectID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnObjectActivate", GuidToBstr(pInfo->guidApp));

		WCHAR szCtxtID[32], szObjID[32];

		wsprintfW(szCtxtID,L"%#016I64X", CtxtID);	
		m_pSpy->AddParamValueToList(L"CtxtID", szCtxtID);
		wsprintfW(szObjID,L"%#016I64X", ObjectID);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjID);

		IF_AUDIT_DO(OnObjectActivate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp), szCtxtID, szObjID);
		return S_OK;
	}

};

#endif //__ObjSub_H__
