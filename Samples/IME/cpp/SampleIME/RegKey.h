// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class CRegKey
{
public:
    CRegKey();
    ~CRegKey();

    HKEY GetHKEY();

    LONG Create(_In_ HKEY hKeyPresent, _In_ LPCWSTR pwszKeyName,
        _In_reads_opt_(255) LPWSTR pwszClass = REG_NONE,
        DWORD dwOptions = REG_OPTION_NON_VOLATILE,
        REGSAM samDesired = KEY_READ | KEY_WRITE,
        _Inout_ LPSECURITY_ATTRIBUTES lpSecAttr = nullptr,
        _Out_opt_ LPDWORD lpdwDisposition = nullptr);

    LONG Open(_In_ HKEY hKeyParent, _In_ LPCWSTR pwszKeyName,
        REGSAM samDesired = KEY_READ | KEY_WRITE);

    LONG Close();

    LONG DeleteSubKey(_In_ LPCWSTR pwszSubKey);
    LONG RecurseDeleteKey(_In_ LPCWSTR pwszSubKey);
    LONG DeleteValue(_In_ LPCWSTR pwszValue);

    LONG QueryStringValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(*pnChars) LPWSTR pwszValue, _Inout_ ULONG *pnChars);
    LONG SetStringValue(_In_opt_ LPCWSTR pwszValueName, _In_ LPCWSTR pwszValue, DWORD dwType = REG_SZ);

    LONG QueryDWORDValue(_In_opt_ LPCWSTR pwszValueName, _Out_ DWORD &dwValue);
    LONG SetDWORDValue(_In_opt_ LPCWSTR pwszValueName, DWORD dwValue);

    LONG QueryBinaryValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(cbData) BYTE* lpData, DWORD cbData);
    LONG SetBinaryValue(_In_opt_ LPCWSTR pwszValueName, _In_reads_(cbData) BYTE* lpData, DWORD cbData);

private:
    HKEY  _keyHandle;
};
