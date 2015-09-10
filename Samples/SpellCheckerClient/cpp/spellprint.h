// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// spellprint.h : Implementation of helper functions to print various spell checking information

#pragma once

#include "util.h"

#include <stdio.h>
#include <objidl.h>
#include <strsafe.h>
#include <spellcheck.h>

inline HRESULT PrintAvailableLanguages(_In_ ISpellCheckerFactory* spellCheckerFactory)
{
    IEnumString* enumLanguages = nullptr;
    HRESULT hr = spellCheckerFactory->get_SupportedLanguages(&enumLanguages);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Available languages:\n");
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintEnumString(enumLanguages, nullptr);
    }

    if (nullptr != enumLanguages)
    {
        enumLanguages->Release();
    }

    PrintErrorIfFailed(L"PrintAvailableLanguages", hr);
    return hr;
}

inline HRESULT PrintSpellingError(_In_ ISpellChecker* spellChecker, _In_ PCWSTR text, _In_ ISpellingError* spellingError)
{
    ULONG startIndex = 0;
    ULONG errorLength = 0;
    CORRECTIVE_ACTION correctiveAction = CORRECTIVE_ACTION_NONE;

    HRESULT hr = spellingError->get_StartIndex(&startIndex);
    if (SUCCEEDED(hr))
    {
        hr = spellingError->get_Length(&errorLength);
    }

    if(SUCCEEDED(hr))
    {
        hr = spellingError->get_CorrectiveAction(&correctiveAction);
    }

    if (SUCCEEDED(hr))
    {
        wchar_t misspelled[MAX_PATH];
        hr = StringCchCopyN(misspelled, ARRAYSIZE(misspelled), text + startIndex, errorLength);

        if (SUCCEEDED(hr))
        {
            wprintf(L"%s [%u, %u] is misspelled. ", misspelled, startIndex, startIndex + errorLength - 1);

            if(CORRECTIVE_ACTION_GET_SUGGESTIONS == correctiveAction)
            {
                wprintf(L"Suggestions:\n");
                IEnumString* enumSuggestions = nullptr;
                hr = spellChecker->Suggest(misspelled, &enumSuggestions);
                if (SUCCEEDED(hr))
                {
                    hr = PrintEnumString(enumSuggestions, L"\t");
                    wprintf(L"\n");
                    enumSuggestions->Release();
                }
            }
            else if (CORRECTIVE_ACTION_REPLACE == correctiveAction)
            {
                wprintf(L"It should be autocorrected to:\n");
                PWSTR replacement = nullptr;
                hr = spellingError->get_Replacement(&replacement);
                if (SUCCEEDED(hr))
                {
                    wprintf(L"\t%s\n\n", replacement);
                    CoTaskMemFree(replacement);
                }
            }
            else if (CORRECTIVE_ACTION_DELETE == correctiveAction)
            {
                wprintf(L"It should be deleted.\n\n");
            }
            else
            {
                wprintf(L"Invalid corrective action.\n\n");
            }
        }
    }

    PrintErrorIfFailed(L"PrintSpellingError", hr);
    return hr;
}

inline HRESULT PrintSpellingErrors(_In_ ISpellChecker* spellChecker, _In_ PCWSTR text, _Inout_ IEnumSpellingError* enumSpellingError)
{
    HRESULT hr = S_OK;
    size_t numErrors = 0;
    while (S_OK == hr)
    {
        ISpellingError* spellingError = nullptr;
        hr = enumSpellingError->Next(&spellingError);
        if (S_OK == hr)
        {
            ++numErrors;
            hr = PrintSpellingError(spellChecker, text, spellingError);
            spellingError->Release();
        }
    }

    if (0 == numErrors)
    {
        wprintf(L"No errors.\n\n");
    }

    PrintErrorIfFailed(L"PrintSpellingErrors", hr);
    return (SUCCEEDED(hr) ? S_OK : hr);
}

inline HRESULT PrintLanguage(_In_ ISpellChecker* spellChecker)
{
    PWSTR languageTag = nullptr;
    HRESULT hr = spellChecker->get_LanguageTag(&languageTag);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Language: %s\n\n", languageTag);
        CoTaskMemFree(languageTag);
    }

    PrintErrorIfFailed(L"PrintLanguage", hr);
    return hr;
}

inline HRESULT PrintSpellCheckerIdAndName(_In_ ISpellChecker* spellChecker)
{
    PWSTR spellCheckerId = nullptr;
    PWSTR localizedName = nullptr;
    HRESULT hr = spellChecker->get_Id(&spellCheckerId);

    if (SUCCEEDED(hr))
    {
        hr = spellChecker->get_LocalizedName(&localizedName);
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"Provider: %s (%s)\n\n", spellCheckerId, localizedName);
    }

    if (nullptr != localizedName)
    {
        CoTaskMemFree(localizedName);
    }

    if (nullptr != spellCheckerId)
    {
        CoTaskMemFree(spellCheckerId);
    }

    PrintErrorIfFailed(L"PrintSpellCheckerIdAndName", hr);
    return hr;
}

inline HRESULT PrintOptionHeading(_In_ IOptionDescription* optionDescription)
{
    PWSTR optionHeading = nullptr;
    HRESULT hr = optionDescription->get_Heading(&optionHeading);
    if (SUCCEEDED(hr))
    {
        if (wcslen(optionHeading) > 0)
        {
            wprintf(L"\t%s\n", optionHeading);
        }
        CoTaskMemFree(optionHeading);
    }

    PrintErrorIfFailed(L"PrintOptionHeading", hr);
    return hr;
}

inline HRESULT PrintOptionDescription(_In_ IOptionDescription* optionDescription)
{
    PWSTR description = nullptr;
    HRESULT hr = optionDescription->get_Description(&description);
    if (SUCCEEDED(hr))
    {
        if (wcslen(description) > 0)
        {
            wprintf(L"\t%s\n", description);
        }
        CoTaskMemFree(description);
    }

    PrintErrorIfFailed(L"PrintOptionDescription", hr);
    return hr;
}

inline HRESULT PrintSingleLabel(_Inout_ IEnumString* enumString, _In_ BYTE optionValue)
{
    LPOLESTR label = nullptr;
    HRESULT hr = enumString->Next(1, &label, nullptr);
    if (S_OK == hr)
    {
        PCWSTR optionState = (optionValue == 1) ? L"on" : L"off";
        wprintf(L"\t%s (current %s)\n", label, optionState);
        CoTaskMemFree(label);
    }

    PrintErrorIfFailed(L"PrintSingleLabel", hr);
    return (SUCCEEDED(hr) ? S_OK : hr);
}

inline HRESULT PrintMultipleLabels(_Inout_ IEnumString* enumString, _In_ BYTE optionValue)
{
    HRESULT hr = S_OK;

    for (int i = 0; (S_OK == hr) ; ++i)
    {
        LPOLESTR label = nullptr;
        hr = enumString->Next(1, &label, nullptr);
        if (S_OK == hr)
        {
            PCWSTR currentText = (optionValue == static_cast<BYTE>(i)) ? L"(current)" : L"";
            wprintf(L"\t[%d] %s %s\n", i, label, currentText);
            CoTaskMemFree(label);
        }
    }

    PrintErrorIfFailed(L"PrintMultipleLabels", hr);
    return (SUCCEEDED(hr) ? S_OK : hr);
}

inline HRESULT PrintOptionLabels(_In_ ISpellChecker* spellChecker, _In_ PCWSTR optionId, _In_ IOptionDescription* optionDescription)
{
    BYTE optionValue;
    HRESULT hr = spellChecker->GetOptionValue(optionId, &optionValue);
    if (SUCCEEDED(hr))
    {
        IEnumString* enumLabels = nullptr;
        hr = optionDescription->get_Labels(&enumLabels);

        if (SUCCEEDED(hr))
        {
            bool hasOneLabel;
            hr = HasSingleString(enumLabels, &hasOneLabel);

            if (SUCCEEDED(hr))
            {
                if (hasOneLabel)
                {
                    hr = PrintSingleLabel(enumLabels, optionValue);
                }
                else
                {
                    hr = PrintMultipleLabels(enumLabels, optionValue);
                }
            }

        }

        if (nullptr != enumLabels)
        {
            enumLabels->Release();
        }
    }

    PrintErrorIfFailed(L"PrintOptionLabels", hr);
    return hr;
}

inline HRESULT PrintOption(_In_ ISpellChecker* spellChecker, _In_ PCWSTR optionId)
{
    wprintf(L"\t%s\n", optionId);

    IOptionDescription* optionDescription = nullptr;
    HRESULT hr = spellChecker->GetOptionDescription(optionId, &optionDescription);

    if (SUCCEEDED(hr))
    {
        hr = PrintOptionHeading(optionDescription);
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintOptionDescription(optionDescription);
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintOptionLabels(spellChecker, optionId, optionDescription);
    }

    PrintErrorIfFailed(L"PrintOption", hr);
    return hr;
}

inline HRESULT PrintSpellingOptions(_In_ ISpellChecker* spellChecker)
{
    wprintf(L"Options:\n");
    IEnumString* enumOptionIds = nullptr;
    HRESULT hr = spellChecker->get_OptionIds(&enumOptionIds);
    
    while (S_OK == hr)
    {
        LPOLESTR optionId = nullptr;
        hr = enumOptionIds->Next(1, &optionId, nullptr);
        if (S_OK == hr)
        {
            hr = PrintOption(spellChecker, optionId);
            wprintf(L"\n");
            CoTaskMemFree(optionId);
        }
    }
    wprintf(L"\n");

    if (nullptr != enumOptionIds)
    {
        enumOptionIds->Release();
    }

    PrintErrorIfFailed(L"PrintSpellingOptions", hr);
    return hr;
}


inline HRESULT PrintInfoAndOptions(_In_ ISpellChecker* spellChecker)
{
    HRESULT hr = PrintLanguage(spellChecker);
    if (SUCCEEDED(hr))
    {
        hr = PrintSpellCheckerIdAndName(spellChecker);
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintSpellingOptions(spellChecker);
    }

    PrintErrorIfFailed(L"PrintInfoAndOptions", hr);
    return hr;
}