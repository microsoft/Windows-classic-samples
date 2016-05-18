// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnEnumDisks.cpp
//
// Description:
//	This file implements the OnEnumDisks() routine which 
//		demonstrates simple instance enumeration.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"

#define TIMEOUT -1
// **************************************************************************
//
//	CAdvClientDlg::OnEnumdisks()
//
// Description:
//		Enumerates all the disks on a machine. Demonstrates getting
//		known properties directly.
//
// Parameters:
//		None.
//
// Returns:
//		Nothing.
//
// Globals accessed:
//		None.
//
// Globals modified:
//		None.
//
//===========================================================================
void CAdvClientDlg::OnEnumdisks() 
{
	// these are for enumerating.
	HRESULT  hRes;
	ULONG uReturned;
	IWbemClassObject *pStorageDev = NULL;
	IEnumWbemClassObject *pEnumStorageDevs = NULL;

	// these help get properties.
	VARIANT pVal;
	BSTR propName = NULL;
    CString buf;

	// here's what we're looking for.
	// NOTE: make sure you specify a class that has a Provider()
	//	specified for it in the mof file. All other classes are
	//	abstract and wont be found using HMM_FLAG_SHALLOW. They
	//	will be found using HMM_FLAG_DEEP.
	BSTR className = SysAllocString(L"Win32_LogicalDisk");
	if (!className)
	{		
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

	// setup for the "__RELPATH" system property.
	propName = SysAllocString(L"__RELPATH");
	if (!propName)
	{		
		SysFreeString(className);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

	VariantInit(&pVal);

	m_outputList.ResetContent();
	m_outputList.AddString(_T("working..."));

	TRACE(_T("Going for class\n"));

	//---------------------------
	// get the list of logical storage devices.
    if ((hRes = m_pIWbemServices->CreateInstanceEnum(className,
												WBEM_FLAG_SHALLOW, 
												NULL,
												&pEnumStorageDevs)) == S_OK) 
	{
		TRACE(_T("good enumerator\n"));

		m_outputList.ResetContent();

		uReturned = 1;
		while(uReturned == 1)
		{
			//---------------------------
			// enumerate through the resultset.
			hRes = pEnumStorageDevs->Next(TIMEOUT,
										1,
										&pStorageDev,
										&uReturned);

			TRACE(_T("Next() %d:%s\n"), uReturned, ErrorString(hRes));

			// was one found?
			if((hRes == S_OK) && (uReturned == 1))
			{

				TRACE(_T("Got a device class\n"));

				VariantClear(&pVal);

			
				// Add the path
				if (pStorageDev->Get(propName, 
										0L, 
										&pVal, 
										NULL, NULL) == S_OK) 
				{
					TRACE(_T("Got a device path\n"));
					 buf = V_BSTR(&pVal);
				} 

				// add to the output listbox.
				m_outputList.AddString(buf);

				// cleanup for next loop
				VariantInit(&pVal);

				// Done with this object.
				if (pStorageDev)
				{ 
					pStorageDev->Release();

					// NOTE: pStorageDev MUST be set to NULL for the next all to Next().
					pStorageDev = NULL;
				} 

			} // endif (hRes == S_OK)

        } // endwhile

		TRACE(_T("done enumming: %s\n"), ErrorString(hRes));

		// Done with this enumerator.
		if (pEnumStorageDevs)
		{ 
			pEnumStorageDevs->Release(); 
			pEnumStorageDevs = NULL;
		}
    } 
	else // CreateInstanceEnum() failed.
	{
		TRACE(_T("CreateInstanceEnum() failed: %s\n"), ErrorString(hRes));

	} // endif CreateInstanceEnum()

	TRACE(_T("returning\n"));
}
