// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __ACTIVITYSUB_H__
#define __ACTIVITYSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CActivitySub
Summary: Activity Event Subscriber
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CActivitySub : 
	public CSysLCESub,
	public IComActivityEvents
{
public:
	CActivitySub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CActivitySub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CActivitySub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CActivitySub)
		COM_INTERFACE_ENTRY(IComActivityEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Activity; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComActivityEvents; }

	STDMETHODIMP OnActivityCreate(COMSVCSEVENTINFO * pInfo, REFGUID guidActivity)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityCreate", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);

		m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);

		IF_AUDIT_DO(OnActivityCreate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),bstrGuidActivity);														
									 
		return S_OK;
	}

	STDMETHODIMP OnActivityDestroy(COMSVCSEVENTINFO * pInfo, REFGUID guidActivity)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityDestroy",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);
		m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);

		IF_AUDIT_DO(OnActivityDestroy)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),bstrGuidActivity);
		return S_OK;

	}

	STDMETHODIMP OnActivityEnter(COMSVCSEVENTINFO * pInfo, REFGUID guidCurrent,REFGUID guidEntered,
		DWORD dwThread)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityEnter",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidCurrent = GuidToBstr(guidCurrent);
		m_pSpy->AddParamValueToList(L"guidCurrent", bstrGuidCurrent);
		CComBSTR bstrGuidEntered = GuidToBstr(guidEntered);
		m_pSpy->AddParamValueToList(L"guidEntered", bstrGuidEntered);

		WCHAR szThread[16];
		wsprintfW(szThread,L"%#08X", dwThread);
		m_pSpy->AddParamValueToList(L"dwThread", szThread);

		IF_AUDIT_DO(OnActivityEnter)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),
									bstrGuidCurrent,
									bstrGuidEntered,
									szThread
									);

		return S_OK;
	}

	STDMETHODIMP OnActivityTimeout(COMSVCSEVENTINFO * pInfo,REFGUID guidCurrent,REFGUID guidEntered,DWORD dwThread,
		DWORD dwTimeout)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityTimeout",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidCurrent = GuidToBstr(guidCurrent);
		m_pSpy->AddParamValueToList(L"guidCurrent", bstrGuidCurrent);
		CComBSTR bstrGuidEntered = GuidToBstr(guidEntered);
		m_pSpy->AddParamValueToList(L"guidEntered", bstrGuidEntered);

		WCHAR szThread[16];
		wsprintfW(szThread,L"%#08X", dwThread);
		m_pSpy->AddParamValueToList(L"dwThread", szThread);

		WCHAR szTimeout[16];
		wsprintfW(szTimeout,L"%d", dwTimeout);
		m_pSpy->AddParamValueToList(L"dwTimeout", szTimeout);

		IF_AUDIT_DO(OnActivityTimeout)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),
									bstrGuidCurrent,
									bstrGuidEntered,
									szThread,
									dwTimeout
									);
		return S_OK;
	}

	STDMETHODIMP OnActivityReenter(COMSVCSEVENTINFO * pInfo,REFGUID guidCurrent,DWORD dwThread,DWORD dwCallDepth)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityReEnter",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidCurrent = GuidToBstr(guidCurrent);
		m_pSpy->AddParamValueToList(L"guidCurrent", bstrGuidCurrent);

		WCHAR szThread[16];
		wsprintfW(szThread,L"%#08X", dwThread);
		m_pSpy->AddParamValueToList(L"dwThread", szThread);

		WCHAR szCallDepth[16];
		wsprintfW(szCallDepth,L"%d", dwCallDepth);
		m_pSpy->AddParamValueToList(L"dwCallDepth", szCallDepth);

		IF_AUDIT_DO(OnActivityReenter)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),
									bstrGuidCurrent,							
									szThread,
									dwCallDepth
									);
		return S_OK;
	}

	STDMETHODIMP OnActivityLeave(COMSVCSEVENTINFO * pInfo,REFGUID guidCurrent,REFGUID guidLeft)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityLeave",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidCurrent = GuidToBstr(guidCurrent);
		m_pSpy->AddParamValueToList(L"guidCurrent", bstrGuidCurrent);
		CComBSTR bstrGuidLeft = GuidToBstr(guidLeft);
		m_pSpy->AddParamValueToList(L"guidLeft", bstrGuidLeft);

		IF_AUDIT_DO(OnActivityLeave)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),
									bstrGuidCurrent,							
									bstrGuidLeft						
									);
		return S_OK;
	}

	STDMETHODIMP OnActivityLeaveSame(COMSVCSEVENTINFO * pInfo,REFGUID guidCurrent,DWORD dwCallDepth)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnActivityLeaveSame",  GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidCurrent = GuidToBstr(guidCurrent);
		m_pSpy->AddParamValueToList(L"guidCurrent", bstrGuidCurrent);
		WCHAR id[16];
		wsprintfW(id,L"%d", dwCallDepth);
		m_pSpy->AddParamValueToList(L"dwCallDepth", id);

		IF_AUDIT_DO(OnActivityLeaveSame)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),
									bstrGuidCurrent,							
									dwCallDepth						
									);
		return S_OK;
	}

};

#endif //__ACTIVITYSUB_H__
