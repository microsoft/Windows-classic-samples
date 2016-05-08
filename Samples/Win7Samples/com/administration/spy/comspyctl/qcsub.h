// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __QCSUB_H__
#define __QCSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CQCSub
Summary: Queued Components Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CQCSub : 
	public CSysLCESub,
	public IComQCEvents
{
public:
	CQCSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CQCSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CQCSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CQCSub)
		COM_INTERFACE_ENTRY(IComQCEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return QC; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComQCEvents; }

	STDMETHODIMP OnQCRecord
	   (COMSVCSEVENTINFO * pInfo,
		ULONG64 objid,
		WCHAR szQueue[60],
		REFGUID guidMsgId,
		REFGUID guidWorkFlowId,
		HRESULT msmqhr)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCRecord", GuidToBstr(pInfo->guidApp));


		CComBSTR bstrGuidMsgId = GuidToBstr(guidMsgId);
		CComBSTR bstrGuidWorkFlowId = GuidToBstr(guidWorkFlowId);

		WCHAR szObjectID[32];
		wsprintfW(szObjectID, L"%#016I64X", objid);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

		m_pSpy->AddParamValueToList(L"Queue", szQueue);
		m_pSpy->AddParamValueToList(L"MsgId", bstrGuidMsgId);
		m_pSpy->AddParamValueToList(L"WorkflowId", bstrGuidWorkFlowId);

		WCHAR szHr[16];
        wsprintfW(szHr, L"%#08X", msmqhr);
		m_pSpy->AddParamValueToList(L"MSMQ HR", szHr);

		IF_AUDIT_DO(OnQCRecord)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 
								szObjectID,
								szQueue,
								bstrGuidMsgId,
								bstrGuidWorkFlowId,
								msmqhr);
		
		return S_OK;
	}

	STDMETHODIMP OnQCQueueOpen
	   (COMSVCSEVENTINFO * pInfo,
		WCHAR szQueue[60],
		ULONG64 QueueID,
		HRESULT msmqhr)
	{
		
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCQueueOpen", GuidToBstr(pInfo->guidApp));
		m_pSpy->AddParamValueToList(L"Queue", szQueue);

		WCHAR szQueueID[32];
		wsprintfW(szQueueID, L"%#016I64X", QueueID);
		m_pSpy->AddParamValueToList(L"QueueID", szQueueID);

		WCHAR p1[16];
        wsprintfW(p1, L"%#08X", msmqhr);
		m_pSpy->AddParamValueToList(L"MSMQ HR", p1);

		IF_AUDIT_DO(OnQCQueueOpen)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 							
									szQueue,
									szQueueID,							
									msmqhr);
		
		return S_OK;
	}
	STDMETHODIMP OnQCReceive
	   (COMSVCSEVENTINFO * pInfo,
		ULONG64 QueueID,
		REFGUID guidMsgId,
		REFGUID guidWorkFlowId,
		HRESULT msmqhr)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCReceive", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidMsgId = GuidToBstr(guidMsgId);
		CComBSTR bstrGuidWorkFlowId = GuidToBstr(guidWorkFlowId);

			
		WCHAR szQueueID[32];
		wsprintfW(szQueueID, L"%#016I64X", QueueID);
		m_pSpy->AddParamValueToList(L"QueueID", szQueueID);

		m_pSpy->AddParamValueToList(L"MsgId", bstrGuidMsgId);
		m_pSpy->AddParamValueToList(L"WorkflowId", bstrGuidWorkFlowId);
		
		WCHAR p1[16];
        wsprintfW(p1, L"%#08X", msmqhr);
		m_pSpy->AddParamValueToList(L"MSMQ HR", p1);


		IF_AUDIT_DO(OnQCReceive)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 														
									szQueueID,	
									bstrGuidMsgId,
									bstrGuidWorkFlowId,
									msmqhr);
		return S_OK;
	}

	STDMETHODIMP OnQCReceiveFail
	   (COMSVCSEVENTINFO * pInfo,
		ULONG64 QueueID,
		HRESULT msmqhr)
	{
		
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCReceiveFail", GuidToBstr(pInfo->guidApp));

		WCHAR szQueueID[32];
		wsprintfW(szQueueID, L"%#016I64X", QueueID);
		m_pSpy->AddParamValueToList(L"QueueID", szQueueID);

		WCHAR p1[16];
        wsprintfW(p1, L"%#08X", msmqhr);
		m_pSpy->AddParamValueToList(L"MSMQ HR", p1);

		IF_AUDIT_DO(OnQCReceiveFail)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 														
									szQueueID,									
									msmqhr);
		return S_OK;
	}
	STDMETHODIMP OnQCMoveToReTryQueue
	   (COMSVCSEVENTINFO * pInfo,
		REFGUID guidMsgId,
		REFGUID guidWorkFlowId,
		ULONG RetryIndex)
	{
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCMoveToReTryQueue", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidMsgId = GuidToBstr(guidMsgId);
		CComBSTR bstrGuidWorkFlowId = GuidToBstr(guidWorkFlowId);	
		
		m_pSpy->AddParamValueToList(L"MsgId", bstrGuidMsgId);
		m_pSpy->AddParamValueToList(L"WorkflowId", bstrGuidWorkFlowId);

		WCHAR p1[16];
        wsprintfW(p1, L"%#08X", RetryIndex);
		m_pSpy->AddParamValueToList(L"ReTry Index", p1);

		IF_AUDIT_DO(OnQCMoveToReTryQueue)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 														
									bstrGuidMsgId,
									bstrGuidWorkFlowId,
									RetryIndex);
									
		return S_OK;
	}
	STDMETHODIMP OnQCMoveToDeadQueue
	   (COMSVCSEVENTINFO * pInfo,
		REFGUID guidMsgId,
		REFGUID guidWorkFlowId)
	{
		CComBSTR bstrGuidMsgId = GuidToBstr(guidMsgId);
		CComBSTR bstrGuidWorkFlowId = GuidToBstr(guidWorkFlowId);
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCMoveToDeadQueue", GuidToBstr(pInfo->guidApp));
		m_pSpy->AddParamValueToList(L"MsgId", bstrGuidMsgId);
		m_pSpy->AddParamValueToList(L"WorkflowId", bstrGuidWorkFlowId);

		IF_AUDIT_DO(OnQCMoveToDeadQueue)(pInfo->perfCount, GuidToBstr(pInfo->guidApp), 														
										bstrGuidMsgId,
										bstrGuidWorkFlowId);
									
									
		return S_OK;
	}

	STDMETHODIMP OnQCPlayback
	   (COMSVCSEVENTINFO * pInfo,
		ULONG64 objid,
		REFGUID guidMsgId,
		REFGUID guidWorkFlowId,
		HRESULT msmqhr)
	{
		
		m_pSpy->AddEventToList(pInfo->perfCount, L"OnQCPlayback", GuidToBstr(pInfo->guidApp));

		CComBSTR bstrGuidMsgId = GuidToBstr(guidMsgId);
		CComBSTR bstrGuidWorkFlowId = GuidToBstr(guidWorkFlowId);

		WCHAR szObjectID[32];
		wsprintfW(szObjectID, L"%#016I64X", objid);
		m_pSpy->AddParamValueToList(L"ObjectID", szObjectID);

		m_pSpy->AddParamValueToList(L"MsgId", bstrGuidMsgId);
		m_pSpy->AddParamValueToList(L"WorkflowId", bstrGuidWorkFlowId);

		WCHAR p1[16];
        wsprintfW(p1, L"%#08X", msmqhr);
		m_pSpy->AddParamValueToList(L"MSMQ HR", p1);

		IF_AUDIT_DO(OnQCPlayback)(pInfo->perfCount, GuidToBstr(pInfo->guidApp),
										szObjectID,
										bstrGuidMsgId,
										bstrGuidWorkFlowId,
										msmqhr);
		
		return S_OK;
	}

};

#endif //__QCSUB_H__
