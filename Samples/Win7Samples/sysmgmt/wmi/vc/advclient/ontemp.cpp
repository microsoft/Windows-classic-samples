// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnTemp.cpp
//
// Description:
//	This file implements the register/unregister of temporary
//		event consumers. 
// 
// History:
//
// **************************************************************************
#include "stdafx.h"
#include "AdvClientDlg.h"

BOOL CAdvClientDlg::OnTempRegister() 
{
	HRESULT hr;

	if(EnsureOfficeNamespace())
	{

		//create a thread that would call ExecNotificationQuery() semi-synchronously and
		//poll for events


		hr = CoMarshalInterThreadInterfaceInStream(IID_IWbemServices,
													m_pOfficeService, 
													&m_pStream);
		if (FAILED(hr))
		{
			m_eventList.AddString(_T("Failed to marshal IWbemServices pointer"));
			return FALSE;

		}

	    m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, 
								(void *)this, 0, &m_threadId);

		if (m_thread == NULL)
		{
			m_eventList.AddString(_T("Failed to create event polling thread"));
			return FALSE;
		}

		return TRUE;
	}					
		
	return FALSE;
}

void CAdvClientDlg::OnTempUnregister() 
{

	if(EnsureOfficeNamespace())
	{
		StopThread();

		m_eventList.ResetContent();
		m_eventList.AddString(_T("unregistered"));
			

	}
}

void CAdvClientDlg::StopThread()
{
    EnterCriticalSection(&m_critSect);

    if (m_thread != NULL) 
	{
		//m_threadCmd = CT_EXIT;
		SetEvent(m_stopThread);
		WaitForSingleObject(m_ptrReady, 100);

        CloseHandle(m_thread);

        m_thread = NULL;
    }
    LeaveCriticalSection(&m_critSect);
}

// ========================================================

DWORD WINAPI CAdvClientDlg::ThreadProc(LPVOID lpParameter)
{

	CoInitialize(NULL);
	
	CAdvClientDlg * pThis = (CAdvClientDlg *)lpParameter;
	
	BSTR qLang = SysAllocString(L"WQL");
	if (!qLang)
	{
		(pThis->m_eventList).AddString(_T("SysAllocString failed"));
		SetEvent(pThis->m_ptrReady);
		return 1;
	}
	BSTR query = SysAllocString(L"select * from __InstanceCreationEvent where TargetInstance isa \"SAMPLE_OfficeEquipment\"");
	if (!query)
	{
		(pThis->m_eventList).AddString(_T("SysAllocString failed"));
		SetEvent(pThis->m_ptrReady);
		SysFreeString(qLang);
		return 1;
	}

	
	IWbemServices * pServices = NULL;
	(pThis->m_eventList).ResetContent();
	
	HRESULT hr = CoGetInterfaceAndReleaseStream(pThis->m_pStream,
										IID_IWbemServices,
										(void**)&pServices);
	if (FAILED(hr))
	{
		(pThis->m_eventList).AddString(_T("Failed to unmarshal interface: "));
		(pThis->m_eventList).AddString(ErrorString(hr));
		SetEvent(pThis->m_ptrReady);
		return 1;
	}


	(pThis->m_eventList).ResetContent();

	IEnumWbemClassObject * pEnumEvents = NULL;

	// execute the query. 
	hr = pServices->ExecNotificationQuery(qLang, query,
											WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, 
											NULL, &pEnumEvents);

	SysFreeString(qLang);
	SysFreeString(query);
	
	if(hr == WBEM_S_NO_ERROR)
	{
		TRACE(_T("Executed event query\n"));
		(pThis->m_eventList).AddString(_T("Ready for events"));

		while(WaitForSingleObject((pThis->m_stopThread), 10) != WAIT_OBJECT_0) 
		{
			IWbemClassObject * pEvent = NULL;
			ULONG uReturned = 0;
		
			//poll for an event
			hr = pEnumEvents->Next(100,
									1,
									&pEvent,
									&uReturned);
			if (SUCCEEDED(hr) && uReturned > 0)
			{
				DisplayEvents(pThis, 1, &pEvent);
                pEvent->Release();
			}
		}

		//thread terminating:
		pEnumEvents->Release();
		
	}
	else
	{
		TRACE(_T("ExecNotificationQuery() failed %s\n"), ErrorString(hr));
		(pThis->m_eventList).AddString(_T("Failed To Register"));

	} //endif ExecQuery()	

	SetEvent(pThis->m_ptrReady);

	return 0;	
	
}

// ========================================================
STDMETHODIMP CAdvClientDlg::DisplayEvents(CAdvClientDlg *pThis,
									LONG lObjectCount,
								    IWbemClassObject **ppObjArray)
{

	if (!pThis)
	{
		return E_FAIL;
	}
	HRESULT  hRes;
	CString clMyBuff;
	BSTR objName = NULL;
	BSTR propName = NULL;
	VARIANT pVal, vDisp;
	IDispatch *pDisp = NULL;
	IWbemClassObject *tgtInst = NULL;

	VariantInit(&pVal);
	VariantInit(&vDisp);

	objName = SysAllocString(L"TargetInstance");
	if (!objName)
		return E_OUTOFMEMORY;
	propName = SysAllocString(L"Item");
	if (!propName)
	{
		SysFreeString(objName);
		return E_OUTOFMEMORY;
	}

	// walk though the classObjects...
	for (int i = 0; i < lObjectCount; i++)
	{
		// clear my output buffer.
		clMyBuff.Empty();

		// get what was added. This will be an embedded object (VT_DISPATCH). All
		// WBEM interfaces are derived from IDispatch.
		if ((hRes = ppObjArray[i]->Get(objName, 0L, 
										&vDisp, NULL, NULL)) == S_OK) 
		{
			//--------------------------------
			// pull the IDispatch out of the various. Dont cast directly to to IWbemClassObject.
			// it MIGHT work now but a suptle change later will break your code. The PROPER 
			// way is to go through QueryInterface and do all the right Release()'s.
			pDisp = (IDispatch *)V_DISPATCH(&vDisp);

			//--------------------------------
			// ask for the IWbemClassObject which is the embedded object. This will be the
			// instance that was created.
			if(SUCCEEDED(pDisp->QueryInterface(IID_IWbemClassObject, (void **)&tgtInst)))
			{
				//--------------------------------
				// dont need the IDispatch anymore.
				pDisp->Release();

				//--------------------------------
				// get the 'Item' property out of the embedded object.
				if ((hRes = tgtInst->Get(propName, 0L, 
										&pVal, NULL, NULL)) == S_OK) 
				{
					//--------------------------------
					// done with it.
					tgtInst->Release();

					// compose a string for the listbox.
					clMyBuff = _T("SAMPLE_OfficeEquipment Instance added for: ");
					clMyBuff += V_BSTR(&pVal);
				
					// output the buffer.
					(pThis->m_eventList).AddString(clMyBuff);
				}
				else
				{
					TRACE(_T("Get() Item failed %s\n"), ErrorString(hRes));
				}
			}
			else
			{
				TRACE(_T("QI() failed \n"));
			}
		}
		else
		{
			TRACE(_T("Get() targetInst failed %s\n"), ErrorString(hRes));
			(pThis->m_eventList).AddString(_T("programming error"));
		} //endif Get()

	} // endfor

	SysFreeString(propName);
	SysFreeString(objName);
	VariantClear(&pVal);

	return S_OK;
}


