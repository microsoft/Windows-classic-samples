// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SampleSpellCheckProviderFactory.cpp : Implementation of the sample provider factory

#include "SampleSpellCheckProviderFactory.h"
#include "SampleSpellCheckProvider.h"
#include "util.h"
#include "EnumString.h"

const PCWSTR supportedLanguages[] = {L"en-us"};

IFACEMETHODIMP CSampleSpellCheckProviderFactory::get_SupportedLanguages(_COM_Outptr_ IEnumString** value)
{
    return CreateEnumString(supportedLanguages, supportedLanguages + ARRAYSIZE(supportedLanguages), value);
}

IFACEMETHODIMP CSampleSpellCheckProviderFactory::IsSupported(_In_ PCWSTR languageTag, _Out_ BOOL* value)
{
    *value = FALSE;
    for (const PCWSTR* tag = supportedLanguages; tag != supportedLanguages + ARRAYSIZE(supportedLanguages); ++tag)
    {
        if (CaseInsensitiveIsEqual(languageTag, *tag))
        {
            *value = TRUE;
            break;
        }
    }

    return S_OK;
}

IFACEMETHODIMP CSampleSpellCheckProviderFactory::CreateSpellCheckProvider(_In_ PCWSTR languageTag, _COM_Outptr_ ISpellCheckProvider** value)
{
    BOOL isSupported = FALSE;
    HRESULT hr = IsSupported(languageTag, &isSupported);
    if (SUCCEEDED(hr) && !isSupported)
    {
        hr = E_INVALIDARG;
    }

    CSampleSpellCheckProvider* spellProvider = nullptr;
    if (SUCCEEDED(hr))
    {
        hr = CSampleSpellCheckProvider::CreateInstance(languageTag, &spellProvider);
    }

    *value = spellProvider;
    return hr;
}
