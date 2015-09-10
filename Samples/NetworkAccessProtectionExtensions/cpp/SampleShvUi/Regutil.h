// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <shlwapi.h>

#define SHVUI_KEY_ACCESS  KEY_ALL_ACCESS

DWORD ShvuiOpenRegKey(
    _In_z_ LPWSTR pRegKey,
    _Out_ HKEY* pKey);

DWORD ShvuiOpenRegKey(
    _In_z_ const LPWSTR pRegKey,
    _In_ BOOL bCreate,
    _Out_ HKEY* pKey);

DWORD ShvuiOpenRegKey(
    _In_ HKEY hive,
    _In_z_ LPWSTR pRegKey,
    _In_ BOOL bCreate,        
    _Out_ HKEY* pKey);

_Success_(return == 0)
DWORD ShvuiGetRegistryValue(
    _In_z_ LPWSTR pRegKey,
    _In_z_ LPWSTR pValueName,
    _In_ _Pre_satisfies_(valueType == REG_DWORD || valueType == REG_SZ || valueType == REG_MULTI_SZ)
           DWORD valueType,
    _When_(valueType == REG_DWORD, _Out_writes_bytes_(sizeof(DWORD)))
    _When_(valueType == REG_SZ || valueType == REG_MULTI_SZ, _Pre_valid_ _Outptr_result_maybenull_)
           PVOID* pData);

_Success_(return == 0)
DWORD ShvuiGetRegistryValue(
    _In_ HKEY hive,
    _In_z_ LPWSTR pRegKey,
    _In_z_ LPWSTR pValueName,
    _In_ _Pre_satisfies_(valueType == REG_DWORD || valueType == REG_SZ || valueType == REG_MULTI_SZ)
           DWORD valueType,
    _When_(valueType == REG_DWORD, _Out_writes_bytes_(sizeof(DWORD)))
    _When_(valueType == REG_SZ || valueType == REG_MULTI_SZ, _Pre_valid_ _Outptr_result_maybenull_)
           PVOID* pData);

DWORD ShvuiSetRegistryValue(
    _In_z_ const LPWSTR pKey,
    _In_opt_z_ const LPWSTR pSubKey,
    _In_z_ const LPWSTR pValueName,
    _In_ DWORD type,
    _In_reads_(cbData) const BYTE* pData,
    _In_ DWORD cbData);

DWORD ShvuiSetRegistryValue(
    _In_ HKEY hive,
    _In_ const LPWSTR pKey,
    _In_opt_z_ const LPWSTR pSubKey,
    _In_opt_z_ const LPWSTR pValueName,
    _In_ DWORD type,
    _In_reads_(cbData) const BYTE* pData,
    _In_ DWORD cbData);

DWORD ShvuiDeleteRegistryKey(
    _In_z_ const LPWSTR pSubkey);

DWORD ShvuiDeleteRegistryKey(
    _In_ HKEY hive,
    _In_z_ const LPWSTR pSubKey);
