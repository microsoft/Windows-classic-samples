// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "RegKey.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CRegKey::CRegKey()
{
    _keyHandle = nullptr;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CRegKey::~CRegKey()
{
    Close();
}


//---------------------------------------------------------------------
//
// operator
//
//---------------------------------------------------------------------

HKEY CRegKey::GetHKEY()
{
    return _keyHandle;
}

//---------------------------------------------------------------------
//
// Create
//
//---------------------------------------------------------------------

LONG CRegKey::Create(_In_ HKEY hKeyPresent, _In_ LPCWSTR pwszKeyName, _In_reads_opt_(255) LPWSTR pwszClass, DWORD dwOptions, REGSAM samDesired, _Inout_ LPSECURITY_ATTRIBUTES lpSecAttr, _Out_opt_ LPDWORD lpdwDisposition)
{
    DWORD disposition = 0;
    HKEY keyHandle = nullptr;

    LONG res = RegCreateKeyEx(hKeyPresent, pwszKeyName, 0,
        pwszClass, dwOptions, samDesired, lpSecAttr, &keyHandle, &disposition);

    if (lpdwDisposition != nullptr)
    {
        *lpdwDisposition = disposition;
    }

    if (res == ERROR_SUCCESS)
    {
        Close();
        _keyHandle = keyHandle;
    }

    return res;
}

//---------------------------------------------------------------------
//
// Open
//
//---------------------------------------------------------------------

LONG CRegKey::Open(_In_ HKEY hKeyParent, _In_ LPCWSTR pwszKeyName, REGSAM samDesired)
{
    HKEY keyHandle = nullptr;

    LONG res = RegOpenKeyEx(hKeyParent, pwszKeyName, 0, samDesired, &keyHandle);
    if (res == ERROR_SUCCESS)
    {
        Close();
        _keyHandle = keyHandle;
    }
    return res;
}

//---------------------------------------------------------------------
//
// Close
//
//---------------------------------------------------------------------

LONG CRegKey::Close()
{
    LONG res = ERROR_SUCCESS;
    if (_keyHandle)
    {
        res = RegCloseKey(_keyHandle);
        _keyHandle = nullptr;
    }
    return res;
}

//---------------------------------------------------------------------
//
// DeleteSubKey
// RecurseDeleteKey
//
//---------------------------------------------------------------------

LONG CRegKey::DeleteSubKey(_In_ LPCWSTR pwszSubKey)
{
    return RegDeleteKey(_keyHandle, pwszSubKey);
}

LONG CRegKey::RecurseDeleteKey(_In_ LPCWSTR pwszSubKey)
{
    CRegKey key;
    LONG res = key.Open(_keyHandle, pwszSubKey, KEY_READ | KEY_WRITE);
    if (res != ERROR_SUCCESS)
    {
        return res;
    }
    FILETIME time;
    WCHAR subKeyName[256] = {'\0'};
    DWORD subKeyNameSize = ARRAYSIZE(subKeyName);

    while (RegEnumKeyEx(key.GetHKEY(), 0, subKeyName, &subKeyNameSize, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
    {
        subKeyName[ARRAYSIZE(subKeyName)-1] = L'\0';
        res = key.RecurseDeleteKey(subKeyName);
        if (res != ERROR_SUCCESS)
        {
            return res;
        }
        subKeyNameSize = ARRAYSIZE(subKeyName);
    }

    key.Close();

    return DeleteSubKey(pwszSubKey);
}

//---------------------------------------------------------------------
//
// DeleteValue
//
//---------------------------------------------------------------------

LONG CRegKey::DeleteValue(_In_ LPCWSTR pwszValue)
{
    return RegDeleteValue(_keyHandle, pwszValue);
}

//---------------------------------------------------------------------
//
// QueryStingValue
// SetStringValue
//
//---------------------------------------------------------------------

LONG CRegKey::QueryStringValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(*pnChars) LPWSTR pwszValue, _Inout_ ULONG *pnChars)
{
    LONG res = 0;
    DWORD dataType = REG_NONE;
    ULONG pwszValueSize = 0;

    if (pnChars == nullptr)
    {
        return E_INVALIDARG;
    }

    pwszValueSize = (*pnChars)*sizeof(WCHAR);
    *pnChars = 0;

    res = RegQueryValueEx(_keyHandle, pwszValueName, NULL, &dataType, (LPBYTE)pwszValue, &pwszValueSize);
    if (res != ERROR_SUCCESS)
    {
        return res;
    }
    if ((dataType != REG_SZ) && (dataType != REG_EXPAND_SZ))
    {
        return ERROR_INVALID_DATA;
    }

    *pnChars = pwszValueSize / sizeof(WCHAR);

    return ERROR_SUCCESS;
}

LONG CRegKey::SetStringValue(_In_opt_ LPCWSTR pwszValueName, _In_ LPCWSTR pwszValue, DWORD dwType)
{
	size_t lenOfValue = 0;
    if (pwszValue == nullptr)
    {
        return ERROR_INVALID_PARAMETER;
    }

	if (StringCchLength(pwszValue, STRSAFE_MAX_CCH, &lenOfValue) != S_OK)
    {
        return ERROR_INVALID_PARAMETER;
    }
	DWORD len = static_cast<DWORD>(lenOfValue);
    return RegSetValueEx(_keyHandle, pwszValueName, NULL, dwType, (LPBYTE)pwszValue, (++len)*sizeof(WCHAR));
}

//---------------------------------------------------------------------
//
// QueryDWORDValue
// SetDWORDValue
//
//---------------------------------------------------------------------

LONG CRegKey::QueryDWORDValue(_In_opt_ LPCWSTR pwszValueName, _Out_ DWORD &dwValue)
{
    LONG res = 0;
    DWORD dataType = REG_NONE;
    ULONG pwszValueSize = 0;

    pwszValueSize = sizeof(DWORD);
    res = RegQueryValueEx(_keyHandle, pwszValueName, NULL, &dataType, (LPBYTE)(&dwValue), &pwszValueSize);
    if (res != ERROR_SUCCESS)
    {
        return res;
    }
    if (dataType != REG_DWORD)
    {
        return ERROR_INVALID_DATA;
    }

    return ERROR_SUCCESS;
}

LONG CRegKey::SetDWORDValue(_In_opt_ LPCWSTR pwszValueName, DWORD dwValue)
{
    return RegSetValueEx(_keyHandle, pwszValueName, NULL, REG_DWORD, (LPBYTE)(&dwValue), sizeof(DWORD));
}

//---------------------------------------------------------------------
//
// QueryBinaryValue
// SetBinaryValue
//
//---------------------------------------------------------------------

LONG CRegKey::QueryBinaryValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(cbData) BYTE* lpData, DWORD cbData)
{
    LONG res = 0;
    DWORD dataType = REG_NONE;
    ULONG pwszValueSize = 0;

    pwszValueSize = cbData;
    res = RegQueryValueEx(_keyHandle, pwszValueName, NULL, &dataType, lpData, &pwszValueSize);
    if (res != ERROR_SUCCESS)
    {
        return res;
    }
    if (dataType != REG_BINARY)
    {
        return ERROR_INVALID_DATA;
    }
    if (pwszValueSize != cbData)
    {
        return ERROR_INVALID_DATA;
    }

    return ERROR_SUCCESS;
}

LONG CRegKey::SetBinaryValue(_In_opt_ LPCWSTR pwszValueName, _In_reads_(cbData) BYTE* lpData, DWORD cbData)
{
    return RegSetValueEx(_keyHandle, pwszValueName, NULL, REG_BINARY, lpData, cbData);
}
