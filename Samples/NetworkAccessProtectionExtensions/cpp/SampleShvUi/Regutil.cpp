// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "regutil.h"
#include <new>

DWORD ShvuiOpenRegKey(
    _In_z_ LPWSTR pRegKey,
    _Out_ HKEY* pKey)
//
// Calls ShvuiOpenRegKey with bCreate = FALSE
//
{
    return ShvuiOpenRegKey(
        pRegKey,
        FALSE,
        pKey);
}

DWORD ShvuiOpenRegKey(
    _In_z_ const LPWSTR pRegKey,
    _In_ BOOL bCreate,
    _Out_ HKEY* pKey)
//
// Calls ShvuiOpenRegKey for HKEY_LOCAL_MACHINE
//
{
    return ShvuiOpenRegKey(
        HKEY_LOCAL_MACHINE,
        pRegKey,
        bCreate,
        pKey);
}


DWORD ShvuiOpenRegKey(
    _In_ HKEY hive,
    _In_z_ LPWSTR pRegKey,
    _In_ BOOL bCreate,        
    _Out_ HKEY* pKey)
//
// Opens the registry key in the hive specified
// if key not found and bCreate is present we attempt to create the key
//
{
    DWORD result = ERROR_SUCCESS;

    if (hive == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    result = RegOpenKeyEx(
                hive,
                pRegKey,
                0 /* Reserved */,
                SHVUI_KEY_ACCESS,
                pKey);

    if (result != ERROR_SUCCESS)
    {
        if (bCreate == FALSE)
        {
			goto cleanup;
        }
        else
        {
            // Try to create the key
            result = RegCreateKeyEx(
                           hive, 
                           pRegKey, 
                           0, 
                           NULL,
                           REG_OPTION_NON_VOLATILE, 
                           SHVUI_KEY_ACCESS, 
                           NULL,
                           pKey, 
                           NULL);

            if (result != ERROR_SUCCESS)
            {
				goto cleanup;
            }
        }
    }

cleanup:
	if (result != ERROR_SUCCESS)
	{
		pKey = NULL;
	}
	return result;
}

_Success_(return == 0)
DWORD ShvuiGetRegistryValue(
    _In_z_ LPWSTR pRegKey,
    _In_z_ LPWSTR pValueName,
    _In_ _Pre_satisfies_(ValueType == REG_DWORD || ValueType == REG_SZ || ValueType == REG_MULTI_SZ)
           DWORD ValueType,
    _When_(ValueType == REG_DWORD, _Out_writes_bytes_(sizeof(DWORD)))
    _When_(ValueType == REG_SZ || ValueType == REG_MULTI_SZ, _Pre_valid_ _Outptr_result_maybenull_)
           PVOID* pData)
//
// Calls ShvuiGetRegistryValue for HKEY_LOCAL_MACHINE
//
{
    return ShvuiGetRegistryValue(
        HKEY_LOCAL_MACHINE,
        pRegKey,
        pValueName,
        ValueType,
        pData);
}

_Success_(return == 0)
DWORD ShvuiGetRegistryValue(
    _In_ HKEY hive,
    _In_z_ LPWSTR pRegKey,
    _In_z_ LPWSTR pValueName,
    _In_ _Pre_satisfies_(valueType == REG_DWORD || valueType == REG_SZ || valueType == REG_MULTI_SZ)
           DWORD valueType,
    _When_(valueType == REG_DWORD, _Out_writes_bytes_(sizeof(DWORD)))
    _When_(valueType == REG_SZ || valueType == REG_MULTI_SZ, _Pre_valid_ _Outptr_result_maybenull_)
           PVOID* pData)
//
// Retrieves data from the given valued of the reg key. If the regkey or value is not present
// or cannot be accessed return error.
//
// Input: 
// RegKey -- null terminated string for regkey
// ValueName -- null terminated string for name of value
// ValueType -- type of value
// Output:
// Data -- buffer to copy the data -- this function allocates memory for string and binary data.
// Caller must free the buffer.
//
{    
    DWORD result = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DWORD localValueType = 0;
    DWORD valueSize = 0;
    LPWSTR pBuf = NULL;
    BOOL bStringValue = FALSE;

    if (pRegKey == NULL || pValueName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pData = NULL;

    result = ShvuiOpenRegKey(
        hive,
        pRegKey,
        FALSE,
        &hKey);

    if( result != ERROR_SUCCESS ) 
    {
        goto cleanup;
    }

    // successfully opened the key -- now read the value

    //
    // Query DataType and BufferSize.
    //

    result = RegQueryValueEx(
        hKey,
        pValueName,
        0,
        &localValueType,
        NULL,
        &valueSize
    );

    if( result != ERROR_SUCCESS )
    {
        goto cleanup;
    }

    if( localValueType != valueType ) {
        result = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    switch(localValueType)
    {
        case REG_DWORD:
            if (valueSize != sizeof(DWORD))
            {
                result = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
            break;
            
        case REG_SZ :
        case REG_MULTI_SZ:
            bStringValue = TRUE;
            
            if (pData == NULL || *pData != NULL)
            {
                result = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
            //
            // Allocate the memory
            //
            pBuf = new (std::nothrow) wchar_t[valueSize];
            if (pBuf == NULL)
            {
                result = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
            break;
        default:
            result = ERROR_INVALID_PARAMETER;
            break;
    }

    if (result == ERROR_SUCCESS)
    {
        //
        // Read the value to the buffer
        //
        result  = RegQueryValueEx(
            hKey,
            pValueName,
            0,
            &localValueType,
            bStringValue==TRUE?(reinterpret_cast<LPBYTE>(pBuf)):(reinterpret_cast<LPBYTE>(pData)),
            &valueSize);

        if (result != ERROR_SUCCESS)
        {
            if (pBuf != NULL)
            {
                delete[] pBuf;
            }
            goto cleanup;
        }

        if (bStringValue == TRUE && pData != NULL)
        {
           *pData = pBuf;
        }
    }
cleanup:
    return result;        
}

DWORD ShvuiSetRegistryValue(
    _In_z_ const LPWSTR pKey,
    _In_opt_z_ const LPWSTR pSubKey,
    _In_z_ const LPWSTR pValueName,
    _In_ DWORD type,
    _In_reads_(cbData) const BYTE* pData,
    _In_ DWORD cbData)
//
//  Call ShvuiSetRegistryValue with HKEY_LOCAL_MACHINE
//
{
    return ShvuiSetRegistryValue(
                    HKEY_LOCAL_MACHINE,
                    pKey,
                    pSubKey,
                    pValueName,
                    type,
                    pData,
                    cbData);
}


DWORD ShvuiSetRegistryValue(
    _In_ HKEY hive,
    _In_ const LPWSTR pKey,
    _In_opt_z_ const LPWSTR pSubKey,
    _In_opt_z_ const LPWSTR pValueName,
    _In_ DWORD type,
    _In_reads_(cbData) const BYTE* pData,
    _In_ DWORD cbData)
//
// Sets value pData of type type to registry key 
// hive\pKey\pSubKey
// If pSubKey is NULL then value is set to hive\pKey
// If hive\pKey or pSubKey is not present, the keys are created.
//
      
{
    DWORD result = ERROR_SUCCESS;
    HKEY hKey = NULL;	 	 		// handle to the reg key
    HKEY hSubKey = NULL;	 	 	// handle to the reg key   

    if (hive == NULL || pKey == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    // Try to open or create the registry key

    result = ShvuiOpenRegKey(
                    hive,
                    pKey,
                    TRUE,
                    &hKey);
                    

    if ((result != ERROR_SUCCESS) || (hKey == NULL))
    {
        goto cleanup;                    
    }

    if (pSubKey != NULL)
    {
        // Need to open this key as well
        result = ShvuiOpenRegKey(
                hKey,
                pSubKey,
                TRUE,
                &hSubKey);
                    
        
        if ((result != ERROR_SUCCESS) || (hSubKey == NULL))
        {
            goto cleanup;
        }
    }

    // Now set the value (if there is one).

    if (pData != NULL)
    {
        RegSetValueEx(
            (pSubKey == NULL)? hKey:hSubKey, 
            pValueName,      // if this is null, value will be set to the key itself 
            0, 
            type, 
            pData,
            cbData);
    }
		
cleanup:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);  
    }

    if (hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }


    return result;

}

DWORD ShvuiDeleteRegistryKey(
    _In_z_ const LPWSTR pSubKey)
//
// Call ShvuiDeleteRegistryKey with HKEY_LOCAL_MACHINE
//
{
    return ShvuiDeleteRegistryKey(
        HKEY_LOCAL_MACHINE,
        pSubKey);
}

DWORD ShvuiDeleteRegistryKey(
    _In_ HKEY hive,
    _In_z_ const LPWSTR pSubKey)
//
//	Delete an entry in the registry of the form:
//			HKEY_CLASSES_ROOT\pKey\pSubkey = value
//

{
   DWORD result = ERROR_SUCCESS;

    if (hive == NULL || pSubKey == NULL)
    {
        return ERROR_INVALID_PARAMETER;        
    }
   
    // delete the registry key.
    result = SHDeleteKey(
                    hive, 
                    pSubKey);

    return result;
}

