//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLE DB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module COMMON.CPP | Common Routines
//
//--------------------------------------------------------------------

////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////
#define INITGUID
#define DBINITCONSTANTS

//Common Header
#include "Common.h"

////////////////////////////////////////////////////////
// CompareVariant
//
////////////////////////////////////////////////////////
LONG CompareVariant
(
	VARIANT *pVar1,	//@parm [in]: Pointer to the variant in the consumer's buffer.
	VARIANT *pVar2,	//@parm [in]: Pointer to the variant at the backend.
	BOOL fCaseSensitive
)
{
	//Handle NULL cases...
	if(pVar1==NULL || pVar2==NULL)
	{
		if(pVar1 == pVar2)
			return TRUE;
		return FALSE;
	}

	// The variant has to be the same type
	if (V_VT(pVar1) != V_VT(pVar2))
		return FALSE;
	
	// Return FALSE if vt is ORed with VT_RESERVED
	if (V_VT(pVar1) & VT_RESERVED)
		return FALSE;

	// Return TRUE is the vt is VT_EMPTY or VT_NULL
	if (V_VT(pVar1)==VT_EMPTY || V_VT(pVar1)==VT_NULL)
		return TRUE;

	switch(V_VT(pVar1))
	{	
		case VT_UI1:
			return V_UI1(pVar1) == V_UI1(pVar2);
		
		case VT_I2:
			return V_I2(pVar1) == V_I2(pVar2);

		case VT_I4:
			return V_I4(pVar1) == V_I4(pVar2);

		case VT_R4:
			return V_R4(pVar1) == V_R4(pVar2);

		case VT_R8:
			return V_R8(pVar1) == V_R8(pVar2);

		case VT_BOOL:
			return V_BOOL(pVar1) == V_BOOL(pVar2);

		case VT_ERROR:
			return V_ERROR(pVar1) == V_ERROR(pVar2);

		case VT_CY:
			return memcmp(&V_CY(pVar1), &V_CY(pVar2),8)==0;

		case VT_DATE:
			return V_DATE(pVar1) == V_DATE(pVar2);

		case VT_BSTR:
			if(fCaseSensitive)
				return wcscmp(V_BSTR(pVar1), V_BSTR(pVar2));
			else
				return _wcsicmp(V_BSTR(pVar1), V_BSTR(pVar2));

		// As we are not testing OLE object, return FALSE for VT_UNKNOWN
		case VT_UNKNOWN:
			return FALSE;

		// As we are not testing OLE object, return FALSE for VT_DISPATCH
		case VT_DISPATCH:
			return FALSE;

		case VT_I2 | VT_BYREF:
			return *V_I2REF(pVar1) == *V_I2REF(pVar2);

		case VT_I4 | VT_BYREF:
			return *V_I4REF(pVar1) == *V_I4REF(pVar2);

		case VT_R4 | VT_BYREF:
			return *V_R4REF(pVar1) == *V_R4REF(pVar2);

		case VT_R8 | VT_BYREF:
			return *V_R8REF(pVar1) == *V_R8REF(pVar2);

		case VT_BOOL | VT_BYREF:
			return *V_BOOLREF(pVar1) == *V_BOOLREF(pVar2);

		case VT_ERROR | VT_BYREF:
			return *V_ERRORREF(pVar1) == *V_ERRORREF(pVar2);

		case VT_CY | VT_BYREF:
			return memcmp(V_CYREF(pVar1), V_CYREF(pVar2),8)==0;

		case VT_DATE | VT_BYREF:
			return *V_DATEREF(pVar1) == *V_DATEREF(pVar2);

		case VT_BSTR | VT_BYREF:
			if(fCaseSensitive)
				return wcscmp(*V_BSTRREF(pVar1), *V_BSTRREF(pVar2));
			else
				return _wcsicmp(*V_BSTRREF(pVar1), *V_BSTRREF(pVar2));

		// As we are not testing OLE object, return FALSE for VT_UNKNOWN
		case VT_UNKNOWN | VT_BYREF:
			return FALSE;

		// As we are not testing OLE object, return FALSE for VT_DISPATCH
		case VT_DISPATCH | VT_BYREF:
			return FALSE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(REGENTRY* pRegEntry, CHAR* pszValue, ULONG cBytes)
{
	return GetRegEntry(pRegEntry->hRootKey, pRegEntry->szKeyName, pRegEntry->szValueName, pszValue, cBytes);
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, CHAR* pszKeyName, CHAR* pszValueName, CHAR* pszValue, ULONG cBytes)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//no-op, pszValueName is not required
	if(!pszKeyName || !pszValue)
		return E_FAIL;
	
	//Obtain the Key for HKEY_CLASSES_ROOT\"SubKey"
	if(ERROR_SUCCESS != RegOpenKeyEx(hRootKey, pszKeyName, 0, KEY_READ, &hKey))
		goto CLEANUP;
	
	//Obtain the Data for the above key
	if(ERROR_SUCCESS == RegQueryValueEx(hKey, pszValueName, NULL, NULL, (LPBYTE)pszValue, &cBytes))
		hr = S_OK;
	
CLEANUP:
	if(hKey)
		RegCloseKey(hKey);
	
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(REGENTRY* pRegEntry)
{
	return SetRegEntry(pRegEntry->hRootKey, pRegEntry->szKeyName, pRegEntry->szValueName, pRegEntry->szValue);
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, CHAR* pszKeyName, CHAR* pszValueName, CHAR* pszValue)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;
	ULONG dwDisposition;

	//no-op, pszValueName is not required
	if(!pszKeyName || !pszValue)
		return E_FAIL;
	
	//Create the Key for HKEY_CLASSES_ROOT\"SubKey"
    if(ERROR_SUCCESS != RegCreateKeyEx(hRootKey, pszKeyName, 0, NULL, 
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition))
			goto CLEANUP;

	//Set the data for the above key
    if(ERROR_SUCCESS == RegSetValueEx(hKey, pszValueName, 0, REG_SZ,
            (BYTE*)pszValue, (DWORD)strlen(pszValue) + sizeof(CHAR)))
		hr = S_OK;

CLEANUP:
	if(hKey)
		RegCloseKey(hKey);
	
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT DelRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelRegEntry(REGENTRY* pRegEntry)
{
	return DelRegEntry(pRegEntry->hRootKey, pRegEntry->szKeyName);
}

////////////////////////////////////////////////////////
// HRESULT DelRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelRegEntry(HKEY hRootKey, CHAR* pszKeyName)
{
	HKEY hKey = NULL;
	HRESULT hr;

	//no-op
	if(!pszKeyName)
		return E_FAIL;
	
	//Delete the Key for HKEY_CLASSES_ROOT\"SubKey"
	hr = RegDeleteKey(hRootKey, pszKeyName);

	//Entry successfully deleted - return S_OK
	if(hr==ERROR_SUCCESS) 
		return S_OK;

	//Entry not found - return S_FALSE
	if(hr==ERROR_FILE_NOT_FOUND)
		return S_FALSE;

	return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToMBCS(WCHAR* pwsz, CHAR* psz, ULONG cbStrLen)
{
	//Convert the string to MBCS
	INT iResult = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cbStrLen, NULL, NULL);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToWCHAR(CHAR* psz, WCHAR* pwsz, ULONG cbStrLen)
{
	//Convert the string to MBCS
	INT iResult = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cbStrLen);
	return iResult ? S_OK : E_FAIL;
}





























