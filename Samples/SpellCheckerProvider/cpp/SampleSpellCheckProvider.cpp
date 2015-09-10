// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SampleSpellCheckProvider.cpp : Implementation of the sample provider

#include "SampleSpellCheckProvider.h"
#include "util.h"
#include "EnumString.h"
#include "strsafe.h"
#include "engineoptions.h"
#include "OptionDescription.h"
#include "EnumSpellingError.h"

const PCWSTR spellerId = L"samplespell";
const PCWSTR localizedName = L"Sample Spell Checker";

IFACEMETHODIMP CSampleSpellCheckProvider::get_LanguageTag(_Out_ PWSTR* value)
{
    return CoTaskStringAlloc(_languageTag, value);
};

IFACEMETHODIMP CSampleSpellCheckProvider::get_Id(_Out_ PWSTR* value)
{
    return CoTaskStringAlloc(spellerId, value);
}

IFACEMETHODIMP CSampleSpellCheckProvider::get_LocalizedName(_Out_ PWSTR* value)
{
    return CoTaskStringAlloc(localizedName, value);
}

IFACEMETHODIMP CSampleSpellCheckProvider::get_OptionIds(_COM_Outptr_ IEnumString** value)
{
    *value = nullptr;
    PCWSTR optionIds[OptionsStore::MAX_LANGUAGE_OPTIONS];
    size_t numIds;
    HRESULT hr = OptionsStore::GetOptionIdsForLanguage(_languageTag, &numIds, optionIds);

    if (SUCCEEDED(hr))
    {
        hr = CreateEnumString(optionIds, optionIds + numIds, value);
    }

    return hr;
}

IFACEMETHODIMP CSampleSpellCheckProvider::Check(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value)
{
    *value = nullptr;
    CEnumSpellingError* enumSpellingError = nullptr;
    HRESULT hr = CEnumSpellingError::CreateInstance(text, this, &enumSpellingError);

    if (SUCCEEDED(hr))
    {
        *value = enumSpellingError;
    }

    return hr;
}

IFACEMETHODIMP CSampleSpellCheckProvider::Suggest(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value)
{
    *value = nullptr;
    wchar_t suggestions[5][SampleEngine::MAX_WORD_SIZE];
    size_t numSuggestions;
    HRESULT hr = engine.GetSuggestions(word, ARRAYSIZE(suggestions), &numSuggestions, suggestions);

    if (SUCCEEDED(hr))
    {
        PCWSTR suggestionList[] = {suggestions[0], suggestions[1], suggestions[2], suggestions[3], suggestions[4]};
        hr = CreateEnumString(suggestionList, suggestionList + numSuggestions, value);
    }

    return hr;
}

IFACEMETHODIMP CSampleSpellCheckProvider::GetOptionValue(_In_ PCWSTR optionId, _Out_ BYTE* value)
{
    return engine.GetOptionValue(optionId, value);
}

IFACEMETHODIMP CSampleSpellCheckProvider::SetOptionValue(_In_ PCWSTR optionId, BYTE value)
{
    return engine.SetOptionValue(optionId, value);
}

IFACEMETHODIMP CSampleSpellCheckProvider::InitializeWordlist(WORDLIST_TYPE wordlistType, _In_ IEnumString* words)
{
    unsigned int type = wordlistType;
    engine.ClearWordlist(type);

    HRESULT hr = S_OK;
    while (S_OK == hr)
    {
        LPOLESTR lpWord;
        hr = words->Next(1, &lpWord, nullptr);

        if (S_OK == hr)
        {
            hr = engine.AddWordToWordlist(type, lpWord);
            CoTaskMemFree(lpWord);
        }
    }

    return hr;
}

IFACEMETHODIMP CSampleSpellCheckProvider::GetOptionDescription(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value)
{
    *value = nullptr;
    COptionDescription* optionDescription;
    HRESULT hr = COptionDescription::CreateInstance(optionId, &optionDescription);

    if (SUCCEEDED(hr))
    {
        *value = optionDescription;
    }

    return hr;
}

HRESULT CSampleSpellCheckProvider::CreateInstance(_In_ PCWSTR languageTag, _COM_Outptr_ CSampleSpellCheckProvider** spellProvider)
{
    HRESULT hr = AtlHelper::CreateInstance(spellProvider);
    if (SUCCEEDED(hr))
    {
        hr = (*spellProvider)->Init(languageTag);
        if (FAILED(hr))
        {
            (*spellProvider)->Release();
            *spellProvider = nullptr;
        }
    }
    else
    {
        *spellProvider = nullptr;
    }

    return hr;
}

HRESULT CSampleSpellCheckProvider::Init(_In_ PCWSTR languageTag)
{
    engine = SampleEngine(languageTag);
    return StringCchCopy(_languageTag, ARRAYSIZE(_languageTag), languageTag);
}

HRESULT CSampleSpellCheckProvider::EngineCheck(_In_ PCWSTR text, _Out_ SampleEngine::SpellingError* spellingError)
{
    return engine.FindFirstError(text, spellingError);
}
