// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// util.h : Implementation of utility functions

#pragma once

#include <atlcom.h>
#include <winnls.h>
#include <strsafe.h>
#include <objbase.h>

class AtlHelper sealed
{
public:
    // the CreateInstance on CComCoClass starts with a reference count of 0.
    // this helper "corrects" that behavior
    template <typename T>
    static HRESULT CreateInstance(_COM_Outptr_ T** ppInstance)
    {
        *ppInstance = nullptr;

        ATL::CComObject<T>* pInstance;
        HRESULT hr = ATL::CComObject<T>::CreateInstance(&pInstance);
        if (SUCCEEDED(hr))
        {
            pInstance->AddRef();
            *ppInstance = pInstance;
        }
        return hr;
    }
};

inline bool CaseInsensitiveIsEqual(_In_NLS_string_(firstSize) PCWSTR first, _In_NLS_string_(secondSize) PCWSTR second, _In_ int firstSize = -1, _In_ int secondSize = -1)
{
    return (CSTR_EQUAL == CompareStringOrdinal(first, firstSize, second, secondSize, TRUE));
}

inline HRESULT CoTaskStringAlloc(_In_ PCWSTR input, _Outptr_result_nullonfailure_ PWSTR* output)
{
    size_t inputSize = wcslen(input);
    *output = reinterpret_cast<PWSTR>(CoTaskMemAlloc(sizeof(wchar_t) * (inputSize + 1)));
    HRESULT hr = (nullptr == *output) ? E_OUTOFMEMORY : S_OK;
    
    if (SUCCEEDED(hr))
    {
        hr = StringCchCopy(*output, inputSize + 1, input);
        if (FAILED(hr))
        {
            CoTaskMemFree(*output);
            *output = nullptr;
        }
    }

    return hr;
}