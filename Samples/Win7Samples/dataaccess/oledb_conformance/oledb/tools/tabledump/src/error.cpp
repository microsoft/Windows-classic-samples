//-----------------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// @doc
//
// @module ERROR.CPP
//
//-----------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "TableDump.h"
#include "Error.h"
#include <olectl.h>	  // IConnectionPoints interface


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayBindingErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayBindingErrors(HWND hWnd, DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData)
{
	//Display the badstatus for pData operations.
	//(ie:  GetData / SetData / InsertRow)
	HRESULT hr = S_OK;
	ASSERT(pData);

	for(DBCOUNTITEM i=0; i<cBindings; i++)
	{
		ASSERT(rgBindings);
		DBSTATUS dwStatus = STATUS_BINDING(rgBindings[i], pData);
		
		switch(dwStatus)
		{
			case DBSTATUS_S_OK:
			case DBSTATUS_S_ISNULL:
			case DBSTATUS_S_DEFAULT:
				break;

			//Display Status Error to the user...
			default:
				OutputText(
					L"\nBinding Status Errors:\n"
					L"rgBindings[%d]\n\n"
					L"iOrdinal = %d\n"
					L"wType = %s\n"
					L"cbMaxLen = %d\n"
					L"bPrecision = %d\n"
					L"bScale = %d\n\n"
					L"DBSTATUS = %s\n",
					i,
					rgBindings[i].iOrdinal,
					GetDBTypeName(rgBindings[i].wType),
					rgBindings[i].cbMaxLen,
					rgBindings[i].bPrecision,
					rgBindings[i].bScale,
					GetStatusName(dwStatus)
					);
				break;
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayAccessorErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayAccessorErrors(HWND hWnd, DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBBINDSTATUS* rgStatus)
{
	//Display the badstatus for row operations.
	HRESULT hr = S_OK;

	for(DBCOUNTITEM i=0; i<cBindings; i++)
	{
		ASSERT(rgBindings);
		ASSERT(rgStatus);
		
		switch(rgStatus[i])
		{
			case DBBINDSTATUS_OK:
				break;

			//Display BindStatus Error to the user...
			default:
				OutputText(
					L"\nAccessor BindStatus Errors:\n"
					L"rgBindings[%d]\n\n"
					L"iOrdinal = %d\n"
					L"wType = %s\n"
					L"cbMaxLen = %d\n"
					L"bPrecision = %d\n"
					L"bScale = %d\n\n"
					L"DBBINDSTATUS = %s\n",
					i,
					rgBindings[i].iOrdinal,
					GetDBTypeName(rgBindings[i].wType),
					rgBindings[i].cbMaxLen,
					rgBindings[i].bPrecision,
					rgBindings[i].bScale,
					GetBindStatusName(rgStatus[i])
					);
				break;
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayAllProps
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayAllProps(HWND hWnd, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	HRESULT hr = S_OK;
	
	//Trace all properties as well...
	VerifyProperties(hr, cPropSets, rgPropSets, FALSE, TRUE);

	//Display a dialog with the affending properties...
	for(ULONG i=0; i<cPropSets; i++)
	{
		for(ULONG j=0; j<rgPropSets[i].cProperties; j++)
		{
			DBPROPSET* pPropSet = &rgPropSets[i];
			DBPROP* pProp = &pPropSet->rgProperties[j];
			
//			if(pProp->dwStatus != DBPROPSTATUS_OK)
//			{
				WCHAR wszPropSet[MAX_NAME_LEN+1];
				WCHAR wszBuffer[MAX_QUERY_LEN+1];
				WCHAR* pwszPropSet = GetPropSetName(pPropSet->guidPropertySet);
				if(pwszPropSet == NULL)
				{	
					StringFromGUID2(pPropSet->guidPropertySet, wszPropSet, MAX_NAME_LEN);
					pwszPropSet = wszPropSet;
				}

				//Find property Value
				wszBuffer[0] = wEOL;
				VariantToString(&pProp->vValue, wszBuffer, MAX_QUERY_LEN, TRUE);

				//Display the Property
				OutputText
				(
//				    L"\nProperties in Error:\n"
//					L"rgPropSets[%d].rgProperties[%d]\n\n"
					L"  { %s, "
					L"%s (0x%08x), "
					L"%s, "
					L"%s, "
					L"\"%s\" }\n",
					pwszPropSet,
					GetPropertyName(pProp->dwPropertyID, pPropSet->guidPropertySet),
					pProp->dwPropertyID, 
					pProp->dwOptions == DBPROPOPTIONS_REQUIRED ? L"DBPROPOPTIONS_REQUIRED" : L"DBPROPOPTIONS_SETIFCHEAP",
					GetDBTypeName(V_VT(&pProp->vValue)),
					wszBuffer
				);
//			}
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayPropErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayPropErrors(HWND hWnd, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	//Display a dialog with the affending properties...
	HRESULT hr = S_OK;
	
	for(ULONG i=0; i<cPropSets; i++)
	{
		for(ULONG j=0; j<rgPropSets[i].cProperties; j++)
		{
			DBPROPSET* pPropSet = &rgPropSets[i];
			DBPROP* pProp = &pPropSet->rgProperties[j];
			if(pProp->dwStatus != DBPROPSTATUS_OK)
			{
				WCHAR wszPropSet[MAX_NAME_LEN+1];
				WCHAR wszBuffer[MAX_QUERY_LEN+1];
				WCHAR* pwszPropSet = GetPropSetName(pPropSet->guidPropertySet);
				if(pwszPropSet == NULL)
				{	
					StringFromGUID2(pPropSet->guidPropertySet, wszPropSet, MAX_NAME_LEN);
					pwszPropSet = wszPropSet;
				}

				//Find property Value
				wszBuffer[0] = wEOL;
				VariantToString(&pProp->vValue, wszBuffer, MAX_QUERY_LEN, TRUE);

				//Append the Error
				OutputText
				(
				    L"\nProperties in Error:\n"
					L"rgPropSets[%d].rgProperties[%d]\n\n"
					L"guidPropertySet = %s\n"
					L"dwPropertyID = %s (0x%08x)\n"
					L"dwOptions = %s\n"
					L"vValue.vt = %s\n"
					L"vValue = \"%s\"\n\n"
					L"dwStatus = %s\n",
					i,j,
					pwszPropSet,
					GetPropertyName(pProp->dwPropertyID, pPropSet->guidPropertySet),
					pProp->dwPropertyID, 
					pProp->dwOptions == DBPROPOPTIONS_REQUIRED ? L"DBPROPOPTIONS_REQUIRED" : L"DBPROPOPTIONS_SETIFCHEAP",
					GetDBTypeName(V_VT(&pProp->vValue)),
					wszBuffer,
					GetPropStatusName(pProp->dwStatus)
				);
			}
		}
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayPropErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayPropErrors(HWND hWnd, REFIID riid, IUnknown* pIUnknown)
{
	ASSERT(pIUnknown);

	//GetProperties with DBPROPSET_PROPERTIESINERROR.
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBProperties* pIDBProperties = NULL;
	ISessionProperties* pISessionProperties = NULL;
	ICommandProperties* pICommandProperties = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;

	//Setup input DBPROPSET_PROPERTIESINERROR
	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].rgPropertyIDs = NULL;

	//ICommand::GetProperties 
	if(riid == IID_IDBProperties)
	{
		XTESTC(hr = pIUnknown->QueryInterface(IID_IDBProperties, (void**)&pIDBProperties));
		XTESTC(hr = pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets));
	}
	else if(riid == IID_ISessionProperties)
	{
		XTESTC(hr = pIUnknown->QueryInterface(IID_ISessionProperties, (void**)&pISessionProperties));
		XTESTC(hr = pISessionProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets));
	}
	else if(riid == IID_ICommandProperties)
	{
		XTESTC(hr = pIUnknown->QueryInterface(IID_ICommandProperties, (void**)&pICommandProperties));
		XTESTC(hr = pICommandProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets));
	}
	else 
	{
		XTESTC(hr = pIUnknown->QueryInterface(IID_IRowsetInfo, (void**)&pIRowsetInfo));
		XTESTC(hr = pIRowsetInfo->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets));
	}

	//Now delegate to display all properties in error
	XTESTC(hr = DisplayPropErrors(hWnd, cPropSets, rgPropSets));

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pISessionProperties);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pIRowsetInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT GetErrorRecords
//
// Get the error message generated by an OLE DB object
/////////////////////////////////////////////////////////////////////////////
HRESULT GetErrorRecords(HWND hWnd, ULONG* pcRecords, IErrorRecords** ppIErrorRecords)
{
	ASSERT(pcRecords && ppIErrorRecords);
	HRESULT hr;

	//NULL output params
	*pcRecords = 0;
	*ppIErrorRecords = NULL;
	
	ISupportErrorInfo* pISupportErrorInfo = NULL;
	IErrorInfo* pIErrorInfo = NULL;

	//See if this interface supports ErrorInfo
	//If not there is no reason to display any error
	if((hr = GetErrorInfo(0, &pIErrorInfo))==S_OK && pIErrorInfo)
	{
		//IErrorRecords may not be supported on the existing error object.
		//Some other things could have posted an error object (VB) for example...
		TESTC(hr = pIErrorInfo->QueryInterface(IID_IErrorRecords, (void**)ppIErrorRecords));
		XTESTC(hr = (*ppIErrorRecords)->GetRecordCount(pcRecords));
	}
		
CLEANUP:
	SAFE_RELEASE(pISupportErrorInfo);
	SAFE_RELEASE(pIErrorInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT GetSqlErrorInfo
//
// Get the error message generated by an OLE DB object
/////////////////////////////////////////////////////////////////////////////
HRESULT GetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr)
{
	ASSERT(pBstr);
	HRESULT hr = S_OK;

	ISQLErrorInfo* pISQLErrorInfo = NULL;
	LONG lNativeError = 0;

	//Get the Error Records
	if(pIErrorRecords)
	{
		//If there is ErrorInfo, GetSQLInfo for the desired record
		//ISQLErrorInfo is not mandatory
		TESTC(hr = pIErrorRecords->GetCustomErrorObject(iRecord, IID_ISQLErrorInfo, (IUnknown**)&pISQLErrorInfo));
		
		//If there was a CustomErrorObject
		if(pISQLErrorInfo)
			hr = pISQLErrorInfo->GetSQLInfo(pBstr, &lNativeError);
	}

CLEANUP:
	SAFE_RELEASE(pISQLErrorInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayAllErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayAllErrors(HWND hWnd, HRESULT hrActual, CHAR* pszFile, ULONG ulLine)
{
	//Delegate
	if(FAILED(hrActual))
		DisplayAllErrors(hWnd, hrActual, S_OK, pszFile, ulLine);

	return hrActual;
}

////////////////////////////////////////////////////////////////////////
// HRESULT DisplayAllErrors
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayAllErrors(HWND hWnd, HRESULT hrActual, HRESULT hrExpected, CHAR* pszFile, ULONG ulLine)
{
	//By Default we only worry about ErrorInfo if(FAILED(hr))
	//But we may want to worry about it at other times as well?
	if(FAILED(hrActual))
	{
		ULONG cRecords = 0;
		IErrorRecords* pIErrorRecords = NULL;

		//Try to display Extened ErrorInfo
		if((GetErrorRecords(hWnd, &cRecords, &pIErrorRecords))==S_OK && cRecords) 
		{
			DisplayErrorRecords(hWnd, cRecords, pIErrorRecords, pszFile, ulLine);
			SAFE_RELEASE(pIErrorRecords);
			goto CLEANUP;
		}
	}
	
	//If not available, display MSG Box with info
	if(FAILED(hrActual) || (hrActual!=S_OK))
	{
		//display the Error
		OutputText(
			L"\nInterface: %s\nResult: 0x%08x = %s\n\nFile: %S\nLine: %d\n", L"Unknown", hrActual, GetErrorName(hrActual), pszFile, ulLine);
	}

CLEANUP:
	return hrActual;
}


////////////////////////////////////////////////////////////////////////
// HRESULT DisplayErrorRecords
//
/////////////////////////////////////////////////////////////////////////////
HRESULT DisplayErrorRecords(HWND hWnd, ULONG cRecords, IErrorRecords* pIErrorRecords, CHAR* pszFile, ULONG ulLine)
{
	HRESULT hr = S_OK;

	IErrorInfo* pIErrorInfo = NULL;
	BSTR bstrErrorInfo = NULL;
	BSTR bstrSQLInfo = NULL;

	static LCID lcid = GetSystemDefaultLCID(); 

	//Get the Error Records
	if(cRecords && pIErrorRecords)
	{
		LONG lNativeError = 0;
		ERRORINFO ErrorInfo;

		//Loop through the records
		for(ULONG i=0; i<cRecords; i++)
		{
			//GetErrorInfo
			XTESTC(hr = pIErrorRecords->GetErrorInfo(i,lcid,&pIErrorInfo));
				
			//Get the Description
			XTESTC(hr = pIErrorInfo->GetDescription(&bstrErrorInfo));
				
			//Get the Basic ErrorInfo
			XTESTC(hr = pIErrorRecords->GetBasicErrorInfo(i,&ErrorInfo));
			
			//Get the SQL Info
			GetSqlErrorInfo(i, pIErrorRecords, &bstrSQLInfo);

			//Display the Error
			if(bstrSQLInfo)
				OutputText(L"\nInterface: %s\nResult: 0x%08x = %s\n\nIErrorInfo: [%s] %s\n\nFile: %S\nLine: %d\n", GetInterfaceName(ErrorInfo.iid), ErrorInfo.hrError, GetErrorName(ErrorInfo.hrError), bstrSQLInfo, bstrErrorInfo, pszFile, ulLine);
			else
				OutputText(L"\nInterface: %s\nResult: 0x%08x = %s\n\nIErrorInfo: %s\n\nFile: %S\nLine: %d\n", GetInterfaceName(ErrorInfo.iid), ErrorInfo.hrError, GetErrorName(ErrorInfo.hrError), bstrErrorInfo, pszFile, ulLine);

			SAFE_RELEASE(pIErrorInfo);
			SAFE_SYSFREE(bstrErrorInfo);
			SAFE_SYSFREE(bstrSQLInfo);
		}
	}
	

CLEANUP:
	SAFE_RELEASE(pIErrorInfo);
	SAFE_SYSFREE(bstrErrorInfo);
	SAFE_SYSFREE(bstrSQLInfo);
	return hr;
}
