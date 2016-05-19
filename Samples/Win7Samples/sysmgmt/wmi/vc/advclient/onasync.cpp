// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnAsync.cpp
//
// Description:
//	This file implements the ExecQueryAsync() routine and the 
//		CAsyncQuerySink class. 
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"
#include "OnAsync.h"

// **************************************************************************
//
//	CAdvClientDlg::OnEnumservicesasync()
//
// Description:
//		Enumerate the services. Demonstrates ExecQueryAsync() using SQL1 and
//		begin/end enumeration through properties. The setup portion is the
//		same as OnEnumServices() but the 'enumerating' part is in the Object
//		Sink.
//
// Parameters:
//		None.
//
// Returns:
//		nothing.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
void CAdvClientDlg::OnEnumservicesasync() 
{

	AfxMessageBox(__T("Please note that using asynchronous WMI APIs remotely in non-Kerberos domains \
requires that the client application lower its authentication level to RPC_C_AUTHN_LEVEL_NONE, \
which makes it less robust from the security standpoint. Use semi-synchronous access \
to WMI data and events instead.\n This sample sets the authentication level to RPC_C_AUTHN_LEVEL_PKT_PRIVACY, \
which means that the following asynchronous data query may fail if you are connecting remotely in a non-Kerberos environment."));

	HRESULT  hRes;

	BSTR qLang = SysAllocString(L"WQL");
	if (!qLang)
	{
		TRACE(_T("ExecQuery() failed: not enough memory\n"));
		return;
	}

	BSTR query = SysAllocString(L"select * from Win32_Service");
	if (!query)
	{
		TRACE(_T("ExecQuery() failed: not enough memory\n"));
		SysFreeString(qLang);
		return;
	}

	//---------------------------
	// allocate the sink if its not already allocated.
	if(m_pQueryCallback == NULL)
	{
		m_pQueryCallback = new CAsyncQuerySink(&m_outputList);
		if (m_pQueryCallback == NULL)
		{
			TRACE(_T("ExecQuery() failed: not enough memory\n"));
			SysFreeString(qLang);
			return;
		}
		m_pQueryCallback->AddRef();
	}


	m_outputList.ResetContent();
	m_outputList.AddString(_T("working..."));

	//---------------------------
	// execute the query. For *Async, the last parm is a sink object
	// that will be sent the resultset instead of returning the normal
	// enumerator object.
	if((hRes = m_pIWbemServices->ExecQueryAsync(qLang, query,
											0L, NULL,              
											m_pQueryCallback)) == S_OK)
	{
		TRACE(_T("Executed query\n"));

		m_outputList.ResetContent();
	}
	else
	{
		TRACE(_T("ExecQuery() failed %s\n"), ErrorString(hRes));

	} //endif ExecQuery()

	SysFreeString(qLang);
	SysFreeString(query);
	
}
// **************************************************************************
//
//	CAsyncQuerySink::CAsyncQuerySink()
//
// Description:
//		This is the sink that gets called as a result of ExecQueryAsync().
//		It has the obligatory COM functions plus Indicate() and SetStatus()
//		which are documented in more detail below.
//===========================================================================
CAsyncQuerySink::CAsyncQuerySink(CListBox *output)
{
	m_pOutputList = output;
	m_SetStatusCalled = FALSE;
}
// ========================================================
CAsyncQuerySink::~CAsyncQuerySink()
{
	if(!m_SetStatusCalled)
	{
		// NOTE: This happening is indicative of a comm error
		// between the server and this client such as a security
		// violation. CoInitializeSecurity() needs to be called to
		// avoid the security problem.
		TRACE(_T("released before SetStatus() was called\n"));
	}
}
// ========================================================
STDMETHODIMP CAsyncQuerySink::QueryInterface(REFIID riid, LPVOID* ppv)
{
	// we're implementing the IID_IWbemObjectSink interface.
    if(riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = this;

		// you're handing out a copy of yourself so account for it.
        AddRef();
        return S_OK;
    }
    else 
	{
		return E_NOINTERFACE;
	}
}

// ========================================================
ULONG CAsyncQuerySink::AddRef()
{
	// InterlockedIncrement() helps with thread safety.
    return InterlockedIncrement(&m_lRef);
}

// ========================================================
ULONG CAsyncQuerySink::Release()
{
	// InterlockedDecrement() helps with thread safety.
    int lNewRef = InterlockedDecrement(&m_lRef);
	// when all the copies are released...
    if(lNewRef == 0)
    {
		// kill thyself.
        delete this;
    }

    return lNewRef;
}

// **************************************************************************
//
//	CAsyncQuerySink::Indicate()
//
// Description:
//		This method is called to handle the result of the ExecQueryAsync().
//		There are no assumtions about how many objects are passed on each
//		call or how many calls will be made.
// Parameters:
//		lObjectCount (in) - how many objects are being passed.
//		ppObjArray (in) - array of objects from the query.
//
// Returns:
//		S_OK if successful, otherwise the HRESULT of the failed called.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
STDMETHODIMP CAsyncQuerySink::Indicate(LONG lObjectCount,
									   IWbemClassObject **ppObjArray)
{
	HRESULT  hRes;
	WCHAR *pBuf;
	CString clMyBuff;
	BSTR propName = NULL, val = NULL;
	VARIANT varString, pVal;

	VariantInit(&varString);
	VariantInit(&pVal);

	TRACE(_T("Indicate() called\n"));

	//------------------------------
	// walk though the classObjects...
	for (int i = 0; i < lObjectCount; i++)
	{
		// clear my output buffer.
		clMyBuff.Empty();

		//------------------------------
		// enumerate properties.
		if((hRes = ppObjArray[i]->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)) == S_OK)
		{
			//------------------------------
			// try to get the next property.
			while(ppObjArray[i]->Next(0, &propName,   
										&pVal, NULL, NULL) == S_OK)
			{
				// is it one of the 'names' we want?
				if((wcsncmp(propName, L"Name", 4) == 0) ||
					(wcsncmp(propName, L"DisplayName", 11) == 0) ||
					(wcsncmp(propName, L"PathName", 8) == 0))
				{
					// format the property=value/
					clMyBuff += propName;
					clMyBuff += _T("=");
					clMyBuff += ValueToString(&pVal, &pBuf);
					clMyBuff += _T("/");
					free(pBuf); // allocated by ValueToString()

				} //endif wcsncmp()...

				// cleanup stuff used in the Next() loop.
				SysFreeString(propName);
				VariantClear(&pVal);

			} //endwhile

			// did the while loop exit due to an error?
			if(hRes != S_OK)
			{
				TRACE(_T("ppObjArray[i]->Next() failed %s\n"), ErrorString(hRes));
			}
		}
		else
		{
			TRACE(_T("BeginEnumeration() failed %s\n"), ErrorString(hRes));
		} //endif BeginEnumeration()

		// output the buffer.
		m_pOutputList->AddString(clMyBuff);

		//------------------------------
		// free the property iterator workspace.
		ppObjArray[i]->EndEnumeration();

		// no need to release because the caller does the
		// AddRef()/Release()

	} // endfor

	TRACE(_T("walked indication list\n"));

	return S_OK;
}
// **************************************************************************
//
//	CAsyncQuerySink::SetStatus()
//
// Description:
//		Called after all Indication()s have been called. This signals the
//		completion of the query.
//
// Parameters:
//		per WMI requirements.
//
// Returns:
//		S_OK if successful, otherwise the HRESULT of the failed called.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
STDMETHODIMP CAsyncQuerySink::SetStatus(long lFlags,
										HRESULT hResult,
										BSTR strParam,
										IWbemClassObject *pObjParam)
{
	TRACE(_T("SetStatus() called %s\n"), ErrorString(hResult));

	m_SetStatusCalled = TRUE;

	// all the Indication()s worked fine.
	if(hResult == WBEM_NO_ERROR)
	{
		m_pOutputList->AddString(_T("Done Enumerating"));
	}
	else // or not.
	{
		m_pOutputList->AddString(_T("Enumeration Error!!"));
	}

	return S_OK;
}
