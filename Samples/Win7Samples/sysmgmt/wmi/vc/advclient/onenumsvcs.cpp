// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnEnumSvcs.cpp
//
// Description:
//	This file implements the OnEnumservices() routine which 
//		demonstrates the ExecQuery() calls.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"

#define TIMEOUT -1


// **************************************************************************
//
//	CAdvClientDlg::OnEnumservices()
//
// Description:
//		Enumerate the services. Demonstrates ExecQuery() using WQL and
//		begin/end enumeration through properties.
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
void CAdvClientDlg::OnEnumservices() 
{
	HRESULT  hRes;
	BSTR propName = NULL, val = NULL;
	VARIANT pVal;
	WCHAR *pBuf;
	CString clMyBuff, prop;
	ULONG uReturned;

	IWbemClassObject *pService = NULL;
	IEnumWbemClassObject *pEnumServices = NULL;

	BSTR qLang = SysAllocString(L"WQL");
	if (!qLang)
	{		
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

	BSTR query = SysAllocString(L"select * from Win32_Service");
	if (!query)
	{		
		SysFreeString(qLang);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

	m_outputList.ResetContent();
	m_outputList.AddString(_T("working..."));

	//----------------------
	// execute the query.
	if((hRes = m_pIWbemServices->ExecQuery(qLang, query,
										0L, NULL,              
										&pEnumServices)) == S_OK)
	{
		TRACE(_T("Executed query\n"));

		m_outputList.ResetContent();

		//----------------------
		// enumerate through services.
		while(((hRes = pEnumServices->Next(TIMEOUT, 1,
										&pService, &uReturned)) == S_OK) &&
			  (uReturned == 1))
		{
			// clear my output buffer.
			clMyBuff.Empty();

			//----------------------
			// different way to enumerate properties.
			if((hRes = pService->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)) == S_OK)
			{
				//----------------------
				// try to get the next property.
				while(pService->Next(0, &propName,   
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
					}

					// cleanup stuff used in the Next() loop.
					SysFreeString(propName);
					VariantClear(&pVal);
				}

				// did the while loop exit due to an error?
				if(hRes != S_OK)
				{
					TRACE(_T("pService->Next() failed %s\n"), ErrorString(hRes));
				}
			}
			else
			{
				TRACE(_T("BeginEnumeration() failed %s\n"), ErrorString(hRes));
			}

			// output the buffer.
			m_outputList.AddString(clMyBuff);

			//----------------------
			// free the iterator space.
			pService->EndEnumeration();

			// done with the ClassObject
			if (pService)
			{ 
				pService->Release(); 
				pService = NULL;
			}
		} //endwhile Next()

		TRACE(_T("walked query\n"));
		m_outputList.AddString(_T("Done Enumerating"));

		// did the while loop exit due to an error?
		if((hRes != S_OK) && 
		   (hRes != 1))
		{
			TRACE(_T("pEnumServices->Next() failed %s\n"), ErrorString(hRes));
		}

		if (pEnumServices)
		{ 
			pEnumServices->Release(); 
			pEnumServices = NULL;
		}
	}
	else
	{
		TRACE(_T("ExecQuery() failed %s\n"), ErrorString(hRes));

	} //endif ExecQuery()

	SysFreeString(qLang);
	SysFreeString(query);
}
