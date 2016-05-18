//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      Helpers.h
//
//  Abstract:
//      Header for helpers used to format resource strings.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//
// A couple of inline functions that create an HRESULT from
// a Win32 error code without the double-evaluation side effect of
// the HRESULT_FROM_WIN32 macro.
//
// Use ResultFromWin32 in place of HRESULT_FROM_WIN32 if
// the side effects of that macro are unwanted.
// ResultFromLastError was created as a convenience for a
// common idiom.
// You could simply call ResultFromWin32(GetLastError()) yourself.
//
__inline HRESULT ResultFromWin32(__in DWORD dwErr)
{
    return HRESULT_FROM_WIN32(dwErr);
}

__inline HRESULT ResultFromLastError()
{
    return ResultFromWin32(GetLastError());
}

// If HRESULT_FROM_WIN32(GetLastError()) is not an error, make it a generic error.
// This should only be called when you already know the function call failed, because
// it will turn success into failure.
__inline HRESULT ResultFromKnownLastError()
{
    HRESULT hr = ResultFromLastError();
    return (SUCCEEDED(hr) ? E_FAIL : hr);
}

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT AllocStringFromResource(
    __in            HINSTANCE     hInstance,
    __in            UINT          uID,
    __deref_out     PWSTR        *ppwsz);
HRESULT FormatString(
    __in            HINSTANCE    hInstance,
    __in            UINT         uID,
    __deref_out     PWSTR       *ppwszOut,
    ...);
HRESULT FormatString(
    __in            PCWSTR   pwszFormat,
    __deref_out     PWSTR   *ppwszOut,
    ...);
HRESULT FormatStringVA(
    __in            PCWSTR       pwszFormat,
    __deref_out     PWSTR       *ppwszOut,
    __in            va_list      vaParamList);
HRESULT FormatStringVA(
    __in            HINSTANCE    hInstance,
    __in            UINT         uID,
    __deref_out     PWSTR       *ppwszOut,
    __in            va_list      vaParamList);

int CompareSyncMgrID(__in PCWSTR pszFirstID, __in PCWSTR pszSecondID);

