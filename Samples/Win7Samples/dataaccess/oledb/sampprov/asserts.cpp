//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ASSERTS.CPP | Simple Assertion Routines
//
//
#include "headers.h"
#include <time.h>
#include "asserts.h"

//////////////////////////////////////////////////////////////////
// LRESULT wSendMessage
//
//////////////////////////////////////////////////////////////////
LRESULT wSendMessage(HWND hWnd, UINT Msg, WPARAM wParam, WCHAR* pwszBuffer)
{
	CHAR szBuffer[MAX_QUERY_LEN];						  
	szBuffer[0] = EOL;
	
	if(pwszBuffer && Msg != WM_GETTEXT && Msg != CB_GETLBTEXT)
	{
		//Convert to ANSI before sending, since we don't know if this was a GET/SET message
		ConvertToMBCS(pwszBuffer, szBuffer, MAX_QUERY_LEN);
	}


	//Send the message with an ANSI Buffer 
	LRESULT lResult = SendMessageA(hWnd, Msg, (WPARAM)wParam, (LPARAM)szBuffer);

	if(pwszBuffer && Msg == WM_GETTEXT || Msg == CB_GETLBTEXT)
	{
		//Now convert the result into the users WCHAR buffer
		// 64 bit TODO
		ConvertToWCHAR(szBuffer, pwszBuffer, (ULONG)(Msg == WM_GETTEXT ? wParam : MAX_QUERY_LEN));
	}
	return lResult;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cStrLen)
{
	ASSERT(psz);

	//No-op
	if(pwsz==NULL)
	{
		psz[0] = EOL;
		return S_FALSE;
	}

	//Convert the string to MBCS
	//NOTE:  WideCharToMultiByte returns 0 if truncation occurs!  So when iResult = 0
	//you can be assured the amount copied is the maximum length of the buffer...
	INT iResult = WideCharToMultiByte(CP_OEMCP, 0, pwsz, -1, psz, cStrLen, NULL, NULL);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cStrLen)
{
	ASSERT(pwsz);

	//No-op
	if(psz==NULL)
	{
		pwsz[0] = wEOL;
		return S_FALSE;
	}

	//Convert the string to MBCS
	//NOTE:  MultiByteToWideChar returns 0 if truncation occurs!  So when iResult = 0
	//you can be assured the amount copied is the maximum length of the buffer...
	INT iResult = MultiByteToWideChar(CP_OEMCP, 0, psz, -1, pwsz, cStrLen);
	return iResult ? S_OK : E_FAIL;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcProperties, DBPROP** prgProperties)
{
	ASSERT(pcProperties);
	ASSERT(prgProperties);
	HRESULT hr = S_OK;
	
	//no-op case
	if(*pcProperties==0 || *prgProperties==NULL)
		return S_OK;
	
	//Free the inner variants first
	for(ULONG i=0; i<*pcProperties; i++)
	{
		//if DBPROPSTATUS_NOTSUPPORTED then vValue is undefined
		if((*prgProperties)[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
			TESTC(hr = VariantClear(&((*prgProperties)[i].vValue)));
	}
	
CLEANUP:
	//Now NULL the set
	*pcProperties = 0;
	SAFE_FREE(*prgProperties);
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcPropSets, DBPROPSET** prgPropSets)
{
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);
	HRESULT hr = S_OK;
	
	//Loop over all the property sets
	for(ULONG i=0; i<*pcPropSets; i++)
		FreeProperties(&((*prgPropSets)[i].cProperties), &((*prgPropSets)[i].rgProperties));
		
	//Now NULL the outer set
	*pcPropSets = 0;
	SAFE_FREE(*prgPropSets);
	return hr;
}



