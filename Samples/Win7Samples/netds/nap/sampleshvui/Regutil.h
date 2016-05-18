// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include <shlwapi.h>

#define SHVUI_KEY_ACCESS  KEY_ALL_ACCESS

DWORD
ShvuiOpenRegKey(
    __in const LPWSTR pwszRegKey,
    __out HKEY* phKey
    );

DWORD
ShvuiOpenRegKey(
        __in const LPWSTR pwszRegKey,
        BOOL bCreate,
        __out HKEY* phKey
    );

DWORD
ShvuiOpenRegKey(
        HKEY hHive,
        __in LPWSTR pwszRegKey,
        BOOL bCreate,        
        __out HKEY* phKey
    );
        
DWORD
ShvuiGetRegistryValue(
    __in const LPWSTR pwszRegKey,
    __in const LPWSTR pwszValueName,
    DWORD ValueType,
    __deref_out PVOID* pData
    );

DWORD
ShvuiGetRegistryValue(
    HKEY hHive,
    __in const LPWSTR pwszRegKey,
    __in const LPWSTR pwszValueName,
    DWORD dwValueType,
    __deref_out PVOID* pData
    );

DWORD
ShvuiSetRegistryValue(
    __in const LPWSTR pwszKey,
    __in_opt const LPWSTR pwszSubKey,
    __in_opt const LPWSTR pwszValueName,
    DWORD dwType,
    __in_bcount(cbData) const BYTE* pData,
    DWORD cbData
    );

DWORD
ShvuiSetRegistryValue(
    HKEY hHive,
    __in const LPWSTR pwszKey,
    __in_opt const LPWSTR pwszSubKey,
    __in_opt const LPWSTR pwszValueName,
    DWORD dwType,
    __in_bcount(cbData) const BYTE* pData,
    DWORD cbData
    );

DWORD
ShvuiDeleteRegistryKey(
    __in const LPWSTR pwszSubkey
    );

DWORD
ShvuiDeleteRegistryKey(
    HKEY hHive,
    __in const LPWSTR pwszSubkey
    );
