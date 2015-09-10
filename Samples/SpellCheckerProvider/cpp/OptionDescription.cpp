// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// OptionDescription.cpp : Implementation of the option description class

#include "OptionDescription.h"
#include "strsafe.h"
#include "util.h"
#include "engineoptions.h"
#include "EnumString.h"

IFACEMETHODIMP COptionDescription::get_Id(_Out_ PWSTR* value)
{
    return CoTaskStringAlloc(_optionId, value);
}

IFACEMETHODIMP COptionDescription::get_Heading(_Out_ PWSTR* value)
{
    *value = nullptr;
    PCWSTR optionHeading = nullptr;
    HRESULT hr = OptionsStore::GetOptionHeading(_optionId, &optionHeading);

    if (SUCCEEDED(hr))
    {
        hr = CoTaskStringAlloc(optionHeading, value);
    }

    return hr;
}

IFACEMETHODIMP COptionDescription::get_Description(_Out_ PWSTR* value)
{
    *value = nullptr;
    PCWSTR optionDescription = nullptr;
    HRESULT hr = OptionsStore::GetOptionDescription(_optionId, &optionDescription);

    if (SUCCEEDED(hr))
    {
        hr = CoTaskStringAlloc(optionDescription, value);
    }

    return hr;
}

IFACEMETHODIMP COptionDescription::get_Labels(_COM_Outptr_ IEnumString** value)
{
    *value = nullptr;
    PCWSTR optionLabels[OptionsStore::MAX_LABELS];
    size_t numLabels;
    HRESULT hr = OptionsStore::GetOptionLabels(_optionId, &numLabels, optionLabels);

    if (SUCCEEDED(hr))
    {
        hr = CreateEnumString(optionLabels, optionLabels + numLabels, value);
    }

    return hr;
}

HRESULT COptionDescription::CreateInstance(_In_ PCWSTR optionId, _COM_Outptr_ COptionDescription** option)
{
    HRESULT hr = AtlHelper::CreateInstance(option);
    if (SUCCEEDED(hr))
    {
        hr = (*option)->Init(optionId);
        if (FAILED(hr))
        {
            (*option)->Release();
            *option = nullptr;
        }
    }
    else
    {
        *option = nullptr;
    }

    return hr;
}

HRESULT COptionDescription::Init(_In_ PCWSTR optionId)
{
    return StringCchCopy(_optionId, ARRAYSIZE(_optionId), optionId);
}