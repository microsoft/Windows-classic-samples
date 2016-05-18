// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __THREADSUB_H__
#define __THREADSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CThreadSub
Summary: Thread Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CThreadSub : 
	public CSysLCESub,
	public IComThreadEvents
{
public:

	CThreadSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	CThreadSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_TXSINK)
	DECLARE_NOT_AGGREGATABLE(CThreadSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CThreadSub)
		COM_INTERFACE_ENTRY(IComThreadEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Thread; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComThreadEvents; }

	STDMETHODIMP OnThreadStart(COMSVCSEVENTINFO * pInfo, ULONG64 ThreadID, DWORD dwThread, DWORD dwThreadCnt)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadStart", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);
		
		WCHAR szW2KThreadID[16];
        wsprintfW(szW2KThreadID,L"%#08X", dwThread);
		m_pSpy->AddParamValueToList(L"dwThread", szW2KThreadID);

		WCHAR szThreadCnt[16];
		wsprintfW(szThreadCnt,L"%lu", dwThreadCnt);
		m_pSpy->AddParamValueToList(L"dwThreadCnt", szThreadCnt);

		IF_AUDIT_DO(OnThreadStart)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szW2KThreadID,dwThreadCnt);
		return S_OK;
	}

	STDMETHODIMP OnThreadTerminate(COMSVCSEVENTINFO * pInfo, ULONG64 ThreadID, DWORD dwThread, DWORD dwThreadCnt)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadTerminate", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);
		
		WCHAR szW2KThreadID[16];
        wsprintfW(szW2KThreadID,L"%#08X", dwThread);
		m_pSpy->AddParamValueToList(L"dwThread", szW2KThreadID);

		WCHAR szThreadCnt[16];
		wsprintfW(szThreadCnt,L"%lu", dwThreadCnt);
		m_pSpy->AddParamValueToList(L"dwThreadCnt", szThreadCnt);

		IF_AUDIT_DO(OnThreadTerminate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szW2KThreadID,dwThreadCnt);
		return S_OK;
	}

	STDMETHODIMP OnThreadBindToApartment(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 AptID,DWORD dwActCnt,DWORD dwLowCnt)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadBindToApartment", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szAptID[32];
		wsprintfW(szAptID,L"%#016I64X", AptID);
		m_pSpy->AddParamValueToList(L"AptID", szAptID);

		WCHAR szActCnt[16];
		wsprintfW(szActCnt,L"%lu", dwActCnt);
		m_pSpy->AddParamValueToList(L"dwActCnt", szActCnt);

		//dwLowCnt  reserved - currently is 0. 

		IF_AUDIT_DO(OnThreadBindToApartment)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szAptID,dwActCnt);
		
		return S_OK;
	}

	STDMETHODIMP OnThreadUnBind(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 AptID,DWORD dwActCnt)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadUnBind", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szAptID[32];
		wsprintfW(szAptID,L"%#016I64X", AptID);
		m_pSpy->AddParamValueToList(L"AptID", szAptID);

		WCHAR szActCnt[16];
		wsprintfW(szActCnt,L"%lu", dwActCnt);
		m_pSpy->AddParamValueToList(L"dwActCnt", szActCnt);

		//dwLowCnt  reserved - currently is 0. 

		IF_AUDIT_DO(OnThreadUnBind)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szAptID,dwActCnt);
		
		return S_OK;
	}

	STDMETHODIMP OnThreadWorkEnque(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 MsgWorkID,DWORD dwQueueLen)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadWorkEnque", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szMsgWorkID[32];
		wsprintfW(szMsgWorkID,L"%#016I64X", MsgWorkID);
		m_pSpy->AddParamValueToList(L"MsgWorkID", szMsgWorkID);

		WCHAR szQueueLen[16];
		wsprintfW(szQueueLen,L"%lu", dwQueueLen);
		m_pSpy->AddParamValueToList(L"QueueLen", szQueueLen);

		IF_AUDIT_DO(OnThreadWorkEnque)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szMsgWorkID,dwQueueLen);

		return S_OK;
	}

	STDMETHODIMP OnThreadWorkPrivate(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 MsgWorkID)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadWorkPrivate", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szMsgWorkID[32];
		wsprintfW(szMsgWorkID,L"%#016I64X", MsgWorkID);
		m_pSpy->AddParamValueToList(L"MsgWorkID", szMsgWorkID);
		
		IF_AUDIT_DO(OnThreadWorkPrivate)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szMsgWorkID);

		return S_OK;
	}

	STDMETHODIMP OnThreadWorkPublic(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 MsgWorkID,DWORD dwQueueLen)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadWorkPublic", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szMsgWorkID[32];
		wsprintfW(szMsgWorkID,L"%#016I64X", MsgWorkID);
		m_pSpy->AddParamValueToList(L"MsgWorkID", szMsgWorkID);

		WCHAR szQueueLen[16];
		wsprintfW(szQueueLen,L"%lu", dwQueueLen);
		m_pSpy->AddParamValueToList(L"QueueLen", szQueueLen);

		IF_AUDIT_DO(OnThreadWorkPublic)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szMsgWorkID,dwQueueLen);

		return S_OK;
	}

	STDMETHODIMP OnThreadWorkRedirect(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 MsgWorkID,DWORD dwQueueLen,ULONG64 ThreadNum)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadWorkRedirect", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szMsgWorkID[32];
		wsprintfW(szMsgWorkID,L"%#016I64X", MsgWorkID);
		m_pSpy->AddParamValueToList(L"MsgWorkID", szMsgWorkID);

		WCHAR szQueueLen[16];
		wsprintfW(szQueueLen,L"%lu", dwQueueLen);
		m_pSpy->AddParamValueToList(L"QueueLen", szQueueLen);

		WCHAR szThreadNum[32];
		wsprintfW(szThreadNum,L"%#016I64X", ThreadNum);
		m_pSpy->AddParamValueToList(L"ThreadNum", szThreadNum);

		IF_AUDIT_DO(OnThreadWorkRedirect)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szMsgWorkID,dwQueueLen,szThreadNum);	

		return S_OK;
	}

	STDMETHODIMP OnThreadWorkReject(COMSVCSEVENTINFO * pInfo,ULONG64 ThreadID,ULONG64 MsgWorkID,DWORD dwQueueLen)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadWorkReject", GuidToBstr(pInfo->guidApp));

		WCHAR szThreadID[32];
		wsprintfW(szThreadID,L"%#016I64X", ThreadID);	
		m_pSpy->AddParamValueToList(L"ThreadID", szThreadID);

		WCHAR szMsgWorkID[32];
		wsprintfW(szMsgWorkID,L"%#016I64X", MsgWorkID);
		m_pSpy->AddParamValueToList(L"MsgWorkID", szMsgWorkID);

		WCHAR szQueueLen[16];
		wsprintfW(szQueueLen,L"%lu", dwQueueLen);
		m_pSpy->AddParamValueToList(L"QueueLen", szQueueLen);

		IF_AUDIT_DO(OnThreadWorkReject)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szThreadID,szMsgWorkID,dwQueueLen);

		return S_OK;
	}

	STDMETHODIMP OnThreadAssignApartment(COMSVCSEVENTINFO * pInfo, REFGUID guidActivity, ULONG64 AptId)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadAssignApartment", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidActivity = GuidToBstr(guidActivity);
		m_pSpy->AddParamValueToList(L"guidActivity", bstrGuidActivity);

		WCHAR szAptID[32];
		wsprintfW(szAptID,L"%#016I64X", AptId);
		m_pSpy->AddParamValueToList(L"AptID", szAptID);

		IF_AUDIT_DO(OnThreadAssignApartment)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),bstrGuidActivity,szAptID);

		return S_OK;
	}

	STDMETHODIMP OnThreadUnassignApartment(COMSVCSEVENTINFO * pInfo, ULONG64 AptId)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnThreadUnassignApartment", GuidToBstr(pInfo->guidApp));

		WCHAR szAptID[32];
		wsprintfW(szAptID,L"%#016I64X", AptId);
		m_pSpy->AddParamValueToList(L"AptID", szAptID);
		
		IF_AUDIT_DO(OnThreadUnassignApartment)(pInfo->perfCount,GuidToBstr(pInfo->guidApp),szAptID);

		return S_OK;
	}

};

#endif //__THREADSUB_H__
