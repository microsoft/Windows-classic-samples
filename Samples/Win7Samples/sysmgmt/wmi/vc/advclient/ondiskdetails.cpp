// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnDiskDetails.cpp
//
// Description:
//	This file implements the OnDiskDetails() routine which 
//		demonstrates how to enumerate properties on your C:
//		drive.
// 
// History:
//
// **************************************************************************


#include "stdafx.h"
#include "AdvClientDlg.h"

// **************************************************************************
//
//	CAdvClientDlg::OnDiskdetails()
//
// Description:
//		Enumerates the properties of the C: drive using the 'GetNames()'
//		technique. The technique uses safearrays.
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
void CAdvClientDlg::OnDiskdetails() 
{
	HRESULT  hRes;
	long lLower, lUpper, lCount; 
	SAFEARRAY *psaNames = NULL;
	BSTR PropName = NULL;
	VARIANT varString, pVal;
	WCHAR *pBuf;
	CString clMyBuff;

	IWbemClassObject *pDriveInst = NULL;
	IWbemQualifierSet *pQualSet = NULL;

	VariantInit(&varString);
	VariantInit(&pVal);

	m_outputList.ResetContent();
	m_outputList.AddString(_T("working..."));

	//-------------------------------
	// Get the instance for C: drive.
	BSTR driveName = SysAllocString(L"Win32_LogicalDisk.DeviceID=\"C:\"");
	if (!driveName)
	{
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}
	BSTR cimType = SysAllocString(L"CIMTYPE");
	if (!cimType)
	{
		SysFreeString(driveName);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}		
	BSTR keyQual = SysAllocString(L"key");
	if (!keyQual)
	{
		SysFreeString(driveName);
		SysFreeString(cimType);
		TRACE(_T("SysAllocString failed: not enough memory\n"));
		return;
	}

    if((hRes = m_pIWbemServices->GetObject(driveName,
										0L,
										NULL,
										&pDriveInst,
										NULL)) == S_OK)
	{

		m_outputList.ResetContent();

		//-------------------------------
		// Get the property names
		if((hRes = pDriveInst->GetNames(NULL, 
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
				//-----------------------------------------------
				// I'm formatting each property as:
				//   name (type) ==> value
				//-----------------------------------------------

				//-------------------------------
				// get the property name for this element
				if((hRes = SafeArrayGetElement(psaNames, 
												&lCount, 
												&PropName)) == S_OK)
				{
					clMyBuff = PropName;

					// print variable type for property value
					clMyBuff += _T(" (");

					// Get pointer to property qualifiers
					// this mess is due to the fact that system properties don't have qualifiers
					if ((pDriveInst->GetPropertyQualifierSet(PropName, &pQualSet)) == S_OK) 
					{
						// Get and print syntax attribute (if any)
						if ((pQualSet->Get(cimType, 0L, &pVal, NULL)) == S_OK) 
						{
						   clMyBuff += V_BSTR(&pVal);
						} 
						else if (hRes != WBEM_E_NOT_FOUND) 
						{  // some other error
						   TRACE(_T("Could not get syntax qualifier\n"));
						   break;
						}
						VariantClear(&pVal);

						//-------------------------------
						// If this is a key field, print an asterisk
						if(((hRes = pQualSet->Get(keyQual, 
												0L, 
												&pVal, 
												NULL)) == S_OK) && 
							(pVal.boolVal))
						{ // Yes, it's a key
						   clMyBuff += _T(")*");
						} 
						else if (hRes == WBEM_E_NOT_FOUND) 
						{  // not a key qualifier
						   clMyBuff += _T(")");
						} 
						else 
						{ // some other error
						   TRACE(_T("Could not get key qualifier\n"));
						   break;
						}
						// done with the qualifierSet.
						if (pQualSet)
						{ 
							pQualSet->Release(); 
							pQualSet = NULL;
						}
					} 
					else 
					{
						clMyBuff += _T(")");
					} //endif pDriveClass->GetPropertyQualifierSet()

					//-------------------------------
					// Get the value for the property.
					if((hRes = pDriveInst->Get(PropName, 
												0L, 
												&varString, 
												NULL, NULL)) == S_OK) 
					{
						// Print the value
						clMyBuff += _T("   ==> ");
						clMyBuff += ValueToString(&varString, &pBuf);
						
						m_outputList.AddString(clMyBuff);

						free(pBuf); // allocated by ValueToString()
					}
					else
					{
						TRACE(_T("Couldn't get Property Value\n"));
						break;
					} //endif pDriveClass->Get()

					VariantClear(&varString);
					VariantClear(&pVal);
				}
				else // SafeArrayGetElement() failed
				{
					TRACE(_T("Couldn't get safe array element\n"));
					break;
				} //endif SafeArrayGetElement()

			} // endfor

			// cleanup.
			SysFreeString(PropName);
			SysFreeString(keyQual);
			SysFreeString(cimType);
			SafeArrayDestroy(psaNames);
			VariantClear(&varString);
			VariantClear(&pVal);
		}
		else // pDriveClass->GetNames() failed
		{
			TRACE(_T("Couldn't GetNames\n"));
		} //endif pDriveClass->GetNames()

		// done with drive instance.
		if (pDriveInst)
		{ 
			pDriveInst->Release(); 
			pDriveInst = NULL;
		}
	} //endif GetObject()
}
