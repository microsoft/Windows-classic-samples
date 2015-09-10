// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SpellingError.cpp : Implementation of the spelling error class

#include "SpellingError.h"
#include "util.h"
#include "strsafe.h"

CSpellingError::~CSpellingError()
{
    CoTaskMemFree(_replacement);
}

IFACEMETHODIMP CSpellingError::get_StartIndex(_Out_ ULONG* value)
{
    *value = _startIndex;
    return S_OK;
}

IFACEMETHODIMP CSpellingError::get_Length(_Out_ ULONG* value)
{
    *value = _errorLength;
    return S_OK;
}

IFACEMETHODIMP CSpellingError::get_CorrectiveAction(_Out_ CORRECTIVE_ACTION* value)
{
    *value = _correctiveAction;
    return S_OK;
}

IFACEMETHODIMP CSpellingError::get_Replacement(_Out_ PWSTR* value)
{
    return CoTaskStringAlloc(_replacement, value);
}

HRESULT CSpellingError::CreateInstance(_In_ ULONG startIndex, _In_ ULONG errorLength, _In_ CORRECTIVE_ACTION correctiveAction, _In_ PCWSTR replacement, _COM_Outptr_ CSpellingError** spellingError)
{
    HRESULT hr = AtlHelper::CreateInstance(spellingError);
    if (SUCCEEDED(hr))
    {
        hr = (*spellingError)->Init(startIndex, errorLength, correctiveAction, replacement);
        if (FAILED(hr))
        {
            (*spellingError)->Release();
            *spellingError = nullptr;
        }
    }
    else
    {
        *spellingError = nullptr;
    }

    return hr;
}

HRESULT CSpellingError::Init(_In_ ULONG startIndex, _In_ ULONG errorLength, _In_ CORRECTIVE_ACTION correctiveAction, _In_ PCWSTR replacement)
{
    _startIndex = startIndex;
    _errorLength = errorLength;
    _correctiveAction = correctiveAction;

    return CoTaskStringAlloc(replacement, &_replacement);
}
