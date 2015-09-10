// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// util.h : Implementation of utility functions

#pragma once

#include <stdio.h>
#include <winerror.h>
#include <objidl.h>

inline void PrintErrorIfFailed(_In_ PCWSTR functionName, _In_ HRESULT errorCode)
{
    if (FAILED(errorCode))
    {
        wprintf(L"Function %s failed with error code 0x%x\n", functionName, errorCode);
    }
}

inline HRESULT PrintEnumString(_In_ IEnumString* enumString, _In_opt_ PCWSTR prefixText)
{
    HRESULT hr = S_OK;
    while (S_OK == hr)
    {
        LPOLESTR string = nullptr;
        hr = enumString->Next(1, &string, nullptr);

        if (S_OK == hr)
        {
            if (nullptr == prefixText)
            {
                wprintf(L"%s\n", string);
            }
            else
            {
                wprintf(L"%s %s\n", prefixText, string);
            }
            CoTaskMemFree(string);
        }
    }

    PrintErrorIfFailed(L"ListAvailableLanguages", hr);
    return (SUCCEEDED(hr) ? S_OK : hr);
}

HRESULT HasSingleString(_Inout_ IEnumString* enumString, _Out_ bool* value)
{
    LPOLESTR strings[2] = {};
    ULONG count = 0;
    HRESULT hr = enumString->Next(2, strings, &count);
    if (SUCCEEDED(hr))
    {
        for (ULONG i = 0; i < count; ++i)
        {
            CoTaskMemFree(strings[i]);
        }
        *value = (count == 1);
        hr = enumString->Reset();
    }

    PrintErrorIfFailed(L"HasSingleString", hr);
    return (SUCCEEDED(hr) ? S_OK : hr);
}
