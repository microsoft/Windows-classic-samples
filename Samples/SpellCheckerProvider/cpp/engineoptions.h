// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// engineoptions.h : Implementation of functions to get information about spell checking options of the engine

#pragma once

#include <winnls.h>
#include <winerror.h>
#include <winbase.h>
#include "resource.h"
#include "util.h"

// Handle to this component, used to get the resources (localized strings)
extern "C" IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))

const size_t MAX_LANGUAGE_OPTIONS = 2;
const size_t MAX_LABELS = 3;

struct OptionDeclaration
{
    PCWSTR optionId;
    unsigned int headingRid;
    unsigned int descriptionRid;
    unsigned char defaultValue;
    _Field_size_(numLabels) const unsigned int* labelRids;
    _Field_range_(1, MAX_LABELS) size_t numLabels;
};

struct LanguageOptions
{
    PCWSTR languageTag;
    _Field_size_(numOptions) const OptionDeclaration* declarations;
    _Field_range_(1, MAX_LANGUAGE_OPTIONS) size_t numOptions;
};

const unsigned int enusIgnoreRepeatedLabelIds[] = {IDS_IGNOREREPEATED_LABEL};
const unsigned int enusOkletterLabelIds[] = {IDS_OKLETTER_LABEL_A, IDS_OKLETTER_LABEL_B, IDS_OKLETTER_LABEL_F};

const OptionDeclaration enusOptions[] = {{L"samplespell:en-US:ignorerepeated", IDS_IGNOREREPEATED_HEADING, IDS_IGNOREREPEATED_DESCRIPTION, 0, enusIgnoreRepeatedLabelIds, ARRAYSIZE(enusIgnoreRepeatedLabelIds)}
                                        ,{L"samplespell:en-US:okletter", IDS_OKLETTER_HEADING, IDS_OKLETTER_DESCRIPTION, 2, enusOkletterLabelIds, ARRAYSIZE(enusOkletterLabelIds)}};

const LanguageOptions spellingOptions[] = {{L"en-US", enusOptions, ARRAYSIZE(enusOptions)}};

class OptionsStore sealed
{
public:
    static HRESULT GetOptionHeading(_In_ PCWSTR const optionId, _Out_ PCWSTR* optionHeading)
    {
        return GetOptionStringFromResource(optionId, &OptionDeclaration::headingRid, optionHeading);
    }

    static HRESULT GetOptionDescription(_In_ PCWSTR const optionId, _Out_ PCWSTR* optionHeading)
    {
        return GetOptionStringFromResource(optionId, &OptionDeclaration::descriptionRid, optionHeading);
    }

    static HRESULT GetOptionIdsForLanguage(_In_ PCWSTR const languageTag, _Deref_out_range_(1, MAX_LANGUAGE_OPTIONS) _Out_ size_t* numOptions, 
                                           _Out_writes_to_(MAX_LANGUAGE_OPTIONS, *numOptions) PCWSTR* optionIds)
    {
        *numOptions = 1;
        *optionIds = L"";
        const LanguageOptions* optionsList = GetLanguageOptionsList(languageTag);
        HRESULT hr = (nullptr == optionsList) ? E_INVALIDARG : S_OK;

        if (SUCCEEDED(hr))
        {
            *numOptions = optionsList->numOptions;
            _Analysis_assume_(*numOptions > 0);
            for (size_t i = 0; i < *numOptions; ++i)
            {
                optionIds[i] = optionsList->declarations[i].optionId;
            }
        }

        if (FAILED(hr))
        {
            for (size_t i = 0; i < *numOptions; ++i)
            {
                optionIds[i] = L"";
            }
        }

        return hr;
    }

    static HRESULT GetOptionLabels(_In_ PCWSTR const optionId, _Deref_out_range_(1, MAX_LABELS) _Out_ size_t* numLabels, 
                                   _Out_writes_to_(MAX_LABELS, *numLabels) PCWSTR* labels)
    {
        *numLabels = 1;
        *labels = L"";
        const OptionDeclaration* declaration = GetOptionDeclarationFromId(optionId);
        HRESULT hr = (nullptr == declaration) ? E_INVALIDARG : S_OK;

        if (SUCCEEDED(hr))
        {
            *numLabels = declaration->numLabels;
            _Analysis_assume_(*numLabels > 0);
            for (size_t i = 0; SUCCEEDED(hr) && (i < *numLabels); ++i)
            {
                hr = LoadStringFromResource(declaration->labelRids[i], &labels[i]);
            }
        }

        if (FAILED(hr))
        {
            for (size_t i = 0; i < *numLabels; ++i)
            {
                labels[i] = L"";
            }
        }

        return hr;
    }

    static HRESULT GetDefaultOptionValue(_In_ PCWSTR const optionId, _Out_ unsigned char* optionValue)
    {
        const OptionDeclaration* declaration = GetOptionDeclarationFromId(optionId);
        HRESULT hr = (nullptr == declaration) ? E_INVALIDARG : S_OK;
        
        if (SUCCEEDED(hr))
        {
            *optionValue = declaration->defaultValue;
        }

        return hr;
    }

    static int GetOptionIndexInLanguage(_In_ PCWSTR const optionId)
    {
        for (size_t i = 0; i < ARRAYSIZE(spellingOptions); ++i)
        {
            const OptionDeclaration* declaration = GetOptionDeclarationFromId(optionId, spellingOptions[i]);
            if (nullptr != declaration)
            {
                return static_cast<int>(declaration - spellingOptions[i].declarations);
            }
        }

        return -1;
    }

public:
    static const size_t MAX_LANGUAGE_OPTIONS = ::MAX_LANGUAGE_OPTIONS;
    static const size_t MAX_LABELS = ::MAX_LABELS;

private:

    static HRESULT GetOptionStringFromResource(_In_ PCWSTR const optionId, _In_ const unsigned int OptionDeclaration::*pRid, _Out_ PCWSTR* optionString)
    {
        *optionString = nullptr;
        const OptionDeclaration* decl = GetOptionDeclarationFromId(optionId);
        HRESULT hr = (nullptr == decl) ? E_INVALIDARG : S_OK;

        if (SUCCEEDED(hr))
        {
            hr = LoadStringFromResource((*decl).*pRid, optionString);
        }

        return hr;
    }

    static const OptionDeclaration* GetOptionDeclarationFromId(_In_ PCWSTR const optionId)
    {
        for (size_t i = 0; i < ARRAYSIZE(spellingOptions); ++i)
        {
            const OptionDeclaration* declaration = GetOptionDeclarationFromId(optionId, spellingOptions[i]);
            if (nullptr != declaration)
            {
                return declaration;
            }
        }

        return nullptr;
    }

    static const OptionDeclaration* GetOptionDeclarationFromId(_In_ PCWSTR const optionId, _In_ const LanguageOptions& optionsList)
    {
        for (size_t i = 0; i < optionsList.numOptions; ++i)
        {
            if (CaseInsensitiveIsEqual(optionsList.declarations[i].optionId, optionId))
            {
                return &optionsList.declarations[i];
            }
        }

        return nullptr;
    }

    static const LanguageOptions* GetLanguageOptionsList(_In_ PCWSTR const languageTag)
    {
        for (size_t i = 0; i < ARRAYSIZE(spellingOptions); ++i)
        {
            if (CaseInsensitiveIsEqual(spellingOptions[i].languageTag, languageTag))
            {
                return &spellingOptions[i];
            }
        }

        return nullptr;
    }

    static HRESULT LoadStringFromResource(_In_ const int resourceIndex, _Out_ PCWSTR* stringResource)
    { 
        HRESULT hr = S_OK;
        if (0 == LoadString(HINST_THISCOMPONENT, resourceIndex, reinterpret_cast<PWSTR>(stringResource), 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;
            }
        }

        return hr;
    }
};
