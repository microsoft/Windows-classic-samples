// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnDiskPropsDescriptions.cpp
//
// Description:
//	This file implements the OnDiskPropsDescriptions() routine which 
//		lists description qualifiers of a class and its properties.
// 
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "AdvClientDlg.h"

// **************************************************************************
//
//	CAdvClientDlg::OnDiskPropsDescriptions()
//
// Description:
//		OnDiskPropsDescriptions.cpp lists class description and property descriptions
//		for Win32_LogicalDisk class. Note that description qualifiers can be quite lengthy
//		and are normally not retrieved, unless WBEM_FLAG_USE_AMENDED_QUALIFIERS flag 
//		is specified in IWbemServices::GetObject(). 
//
//		Object qualifiers are retieved by IWbemClassObject::GetQualifierSet(). 
//		Property qualifiers are retrieved by IWbemClassObject::GetPropertyQualifierSet() 
//		- you need to supply property name as a parameter.
//
//		Get() method on the IWbemQualifierSet retrives specific qualifier values - 
//		in this case, descriptions.
//
//		Amended qualifiers (such as descriptions) are localizable and 
//		will be displayed in the language that corresponds to the current user 
//		locale on the client machine, as long as the server is able to provide
//		appropriate localized resources.
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


void CAdvClientDlg::OnDiskPropsDescriptions() 
{

	HRESULT  hRes;
	long lLower, lUpper, lCount; 
	SAFEARRAY *psaNames = NULL;
	VARIANT pVal;
	BSTR PropName = NULL;
	CString clMyBuff;

	VariantInit(&pVal);

	IWbemClassObject *pDriveClass = NULL;
	IWbemQualifierSet *pQualSet = NULL;

	m_outputList.ResetContent();
	m_outputList.AddString(_T("working..."));

	BSTR driveClassName = SysAllocString(L"Win32_LogicalDisk");
	if (!driveClassName)
	{
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

	BSTR descr = SysAllocString(L"Description");
	if (!descr)
	{
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		SysFreeString(driveClassName);
		return;
	}

    if((hRes = m_pIWbemServices->GetObject(driveClassName,
										WBEM_FLAG_USE_AMENDED_QUALIFIERS,	// this is necessary to retrieve descriptions
										NULL,
										&pDriveClass,
										NULL)) == S_OK)
	{

		m_outputList.ResetContent();

		//get class description
		hRes = pDriveClass->GetQualifierSet(&pQualSet);
		if (hRes == S_OK) 
		{
			hRes = pQualSet->Get(descr, 0L, &pVal, NULL);
			if (hRes == S_OK) 
			{
			   m_outputList.AddString(CString(V_BSTR(&pVal)));
			} 

			VariantClear(&pVal);
		}
		pQualSet->Release();

		//-------------------------------
		// Get the property names
		if((hRes = pDriveClass->GetNames(NULL, 
										WBEM_FLAG_ALWAYS | 
										WBEM_FLAG_NONSYSTEM_ONLY, 
										NULL, 
										&psaNames)) == S_OK)
		{
			//-------------------------------
			// Get the upper and lower bounds of the Names array
			if((hRes = SafeArrayGetLBound(psaNames, 1, &lLower)) != S_OK) 
			{
				TRACE(_T("Couldn't get safe array lbound\n"));
				SafeArrayDestroy(psaNames);
				return;
			}

			//-------------------------------
			if((hRes = SafeArrayGetUBound(psaNames, 1, &lUpper)) != S_OK) 
			{
				TRACE(_T("Couldn't get safe array ubound\n"));
				SafeArrayDestroy(psaNames);
				return;
			}


			//-------------------------------
			// For all properties...
			for (lCount = lLower; lCount <= lUpper; lCount++) 
			{

				//-------------------------------
				// get the property name for this element
				if((hRes = SafeArrayGetElement(psaNames, 
												&lCount, 
												&PropName)) == S_OK)
				{
				
					// Get pointer to property qualifiers
					// this mess is due to the fact that system properties don't have qualifiers
					if ((pDriveClass->GetPropertyQualifierSet(PropName, &pQualSet)) == S_OK) 
					{
						// Get and print description
						if ((pQualSet->Get(descr, 0L, &pVal, NULL)) == S_OK) 
						{
							m_outputList.AddString(CString(V_BSTR(&pVal)));
						} 
						VariantClear(&pVal);

						// done with the qualifierSet.
						if (pQualSet)
						{ 
							pQualSet->Release(); 
							pQualSet = NULL;
						}
					} 

				}
				else // SafeArrayGetElement() failed
				{
					TRACE(_T("Couldn't get safe array element\n"));
					break;
				} //endif SafeArrayGetElement()

			} // endfor

			// cleanup.
			SysFreeString(PropName);
			SysFreeString(descr);
			SafeArrayDestroy(psaNames);
			VariantClear(&pVal);
		}
		else // pDriveClass->GetNames() failed
		{
			TRACE(_T("Couldn't GetNames\n"));
		} //endif pDriveClass->GetNames()

		// done with drive instance.
		if (pDriveClass)
		{ 
			pDriveClass->Release(); 
			pDriveClass = NULL;
		}
	} //endif GetObject()

	
}
