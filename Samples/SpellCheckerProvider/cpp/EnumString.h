// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// EnumString.h : Declaration of the CEnumString

#pragma once

#include <atlbase.h>
#include "util.h"

typedef CComEnum<IEnumString, &IID_IEnumString, LPOLESTR, ATL::_Copy<LPOLESTR>, ATL::CComMultiThreadModelNoCS> CEnumString;

inline HRESULT CreateEnumString(_In_reads_to_ptr_(end) PCWSTR const* begin, _Notnull_ PCWSTR const* end, _COM_Outptr_ IEnumString** value)
{
    *value = nullptr;
    CEnumString* enumString = nullptr;
    HRESULT hr = AtlHelper::CreateInstance(&enumString);

    if (SUCCEEDED(hr))
    {
        // Need to const_cast, as it isn't possible to use CComEnum with different storage and exposed types
        // Since we're using AtlFlagCopy, Init won't change the input
        hr = enumString->Init(const_cast<LPOLESTR*>(begin), const_cast<LPOLESTR*>(end), nullptr, ATL::AtlFlagCopy);
        if (FAILED(hr))
        {
            enumString->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        *value = enumString;
    }

    return hr;
}


