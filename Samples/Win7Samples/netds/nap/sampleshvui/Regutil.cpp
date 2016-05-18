// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "regutil.h"
#include "precomp.h"
#include <assert.h>

DWORD
ShvuiOpenRegKey(
    __in LPWSTR pwszRegKey,
    __out HKEY* phKey)
//
// Calls ShvuiOpenRegKey with bCreate = FALSE
//
{
    return ShvuiOpenRegKey(
        pwszRegKey,
        FALSE,
        phKey);
}

DWORD
ShvuiOpenRegKey(
        __in const LPWSTR pwszRegKey,
        BOOL bCreate,
        __out HKEY* phKey)
//
// Calls ShvuiOpenRegKey for HKEY_LOCAL_MACHINE
//
{
    return ShvuiOpenRegKey(
        HKEY_LOCAL_MACHINE,
        pwszRegKey,
        bCreate,
        phKey);
}


DWORD
ShvuiOpenRegKey(
        HKEY hHive,
        __in LPWSTR pwszRegKey,
        BOOL bCreate,        
        __out HKEY* phKey)
//
// Opens the registry key in the hive specified
// if key not found and bCreate is present we attempt to create the key
//
{
    DWORD result = ERROR_SUCCESS;

    if (hHive == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    result = RegOpenKeyEx(
                hHive,
                pwszRegKey,
                0 /* Reserved */,
                SHVUI_KEY_ACCESS,
                phKey);

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
                           hHive, 
                           pwszRegKey, 
                           0, 
                           NULL,
                           REG_OPTION_NON_VOLATILE, 
                           SHVUI_KEY_ACCESS, 
                           NULL,
                           phKey, 
                           NULL);

            if (result != ERROR_SUCCESS)
            {
                goto cleanup;
            }
        }
    }

cleanup:
    return result;
}

DWORD
ShvuiGetRegistryValue(
    __in LPWSTR pwszRegKey,
    __in LPWSTR pwszValueName,
    DWORD ValueType,
    __deref_out PVOID* pData)
//
// Calls ShvuiGetRegistryValue for HKEY_LOCAL_MACHINE
//
{
    return ShvuiGetRegistryValue(
        HKEY_LOCAL_MACHINE,
        pwszRegKey,
        pwszValueName,
        ValueType,
        pData);
}

DWORD
ShvuiGetRegistryValue(
    HKEY hHive,
    __in LPWSTR pwszRegKey,
    __in LPWSTR pwszValueName,
    DWORD dwValueType,
    __deref_out PVOID* pData)
//
// Retrives data from the given valued of the reg key. If the regkey or value is not present
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
    DWORD dwLocalValueType = 0;
    DWORD dwValueSize = 0;
    LPWSTR pwszBuf = NULL;
    BOOL bStringValue = FALSE;

    if (pwszRegKey == NULL ||pwszValueName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    result = ShvuiOpenRegKey(
        hHive,
        pwszRegKey,
        FALSE,
        &hKey);

    if( result != ERROR_SUCCESS ) 
    {
        goto cleanup;
    }

    // succesfully opened the key -- now read the value

    //
    // Query DataType and BufferSize.
    //

    result = RegQueryValueEx(
        hKey,
        pwszValueName,
        0,
        &dwLocalValueType,
        NULL,
        &dwValueSize
    );

    if( result != ERROR_SUCCESS )
    {
        goto cleanup;
    }

    if( dwLocalValueType != dwValueType ) {
        result = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    switch(dwLocalValueType)
    {
        case REG_DWORD:
            assert(dwValueSize == sizeof(DWORD));
            break;
            
        case REG_SZ :
        case REG_MULTI_SZ:
            bStringValue = TRUE;
            
            assert(*pData == NULL);
            //
            // Allocate the memory
            //
            pwszBuf = (LPWSTR)new wchar_t[dwValueSize];
            if (pwszBuf == NULL)
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
            pwszValueName,
            0,
            &dwLocalValueType,
            bStringValue==TRUE?((LPBYTE)(pwszBuf)):((LPBYTE)(pData)),
            &dwValueSize);

        if (result != ERROR_SUCCESS)
        {
            if (pwszBuf != NULL)
            {
                delete[] pwszBuf;
            }
            goto cleanup;
        }

        if (bStringValue == TRUE)
        {
           *pData = pwszBuf;
        }
    }
cleanup:
    return result;        
}

DWORD
ShvuiSetRegistryValue(
    __in const LPWSTR pwszKey,
    __in_opt const LPWSTR pwszSubKey,
    __in const LPWSTR pwszValueName,
    DWORD dwType,
    __in_bcount(cbData) const BYTE* pData,
    DWORD cbData)
//
//  Call ShvuiSetRegistryValue with HKEY_LOCAL_MACHINE
//
{
    return ShvuiSetRegistryValue(
                    HKEY_LOCAL_MACHINE,
                    pwszKey,
                    pwszSubKey,
                    pwszValueName,
                    dwType,
                    pData,
                    cbData);
}


DWORD
ShvuiSetRegistryValue(
    HKEY hHive,
    __in const LPWSTR pwszKey,
    __in_opt const LPWSTR pwszSubKey,
    __in_opt const LPWSTR pwszValueName,
    DWORD dwType,
    __in_bcount(cbData) const BYTE* pData,
    DWORD cbData)
//
// Sets value pData of type dwType to registry key 
// hHive\pwszKey\pwszSubkey
// If pwszSubkey is NULL then value is set to hHive\pwszKey
// If hHive\pwszKey or pwszSubkey is not present, the keys are created.
//
      
{
    DWORD result = ERROR_SUCCESS;
    HKEY hKey = NULL;	 	 		// handle to the reg key
    HKEY hSubKey = NULL;	 	 		// handle to the reg key   


    if (hHive == NULL || pwszKey == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    // Try to open or create the registry key

    result = ShvuiOpenRegKey(
                    hHive,
                    pwszKey,
                    TRUE,
                    &hKey);
                    

    if ((result != ERROR_SUCCESS) || (hKey == NULL))
    {
        goto cleanup;                    
    }

    if (pwszSubKey != NULL)
    {
        // Need to open this key as well
        result = ShvuiOpenRegKey(
                hKey,
                pwszSubKey,
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
            (pwszSubKey == NULL)? hKey:hSubKey, 
            pwszValueName,      // if this is null, value will be set to the key itself 
            0, 
            dwType, 
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

DWORD
ShvuiDeleteRegistryKey(
    __in const LPWSTR pwszSubkey)
//
// Call ShvuiDeleteRegistryKey with HKEY_LOCAL_MACHINE
//
{
    return ShvuiDeleteRegistryKey(
        HKEY_LOCAL_MACHINE,
        pwszSubkey);
}

DWORD
ShvuiDeleteRegistryKey(
    HKEY hHive,
    __in const LPWSTR pwszSubKey)
//
//	Delete an entry in the registry of the form:
//			HKEY_CLASSES_ROOT\wszKey\wszSubkey = wszValue
//

{
   DWORD result = ERROR_SUCCESS;

    if (hHive == NULL || pwszSubKey == NULL)
    {
        return ERROR_INVALID_PARAMETER;        
    }
   
    if (pwszSubKey != NULL)
    {
        // delete the registry key.
        result = SHDeleteKey(
                        hHive, 
                        pwszSubKey);
    }

    return result;
}

