// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// EnumSpellingError.cpp : Implementation of the spelling error enumerator class

#include "EnumSpellingError.h"
#include "SpellingError.h"
#include "util.h"
#include "strsafe.h"
#include "sampleengine.h"

_Success_(return == S_OK)
IFACEMETHODIMP CEnumSpellingError::Next(_COM_Outptr_ ISpellingError** value)
{
    *value = nullptr;
    SampleEngine::SpellingError spellingError;

    HRESULT hr = _spellcheckProvider->EngineCheck(_currentTextPosition, &spellingError);
    if (S_FALSE == hr) // no more spelling errors left
    {
        return hr;
    }

    CSpellingError* returnedError = nullptr;
    if (S_OK == hr)
    {
        const size_t indexInOriginal = _currentTextPosition - _text;
        hr = CSpellingError::CreateInstance(static_cast<ULONG>(indexInOriginal + spellingError.startIndex), static_cast<ULONG>(spellingError.errorLength),
                                            static_cast<CORRECTIVE_ACTION>(spellingError.correctiveAction), spellingError.replacement, &returnedError);

        _currentTextPosition += spellingError.startIndex + spellingError.errorLength;
    }

    if (S_OK == hr)
    {
        *value = returnedError;
    }

    return hr;
}

HRESULT CEnumSpellingError::CreateInstance(_In_ PCWSTR text, _In_ CSampleSpellCheckProvider* spellcheckProvider, _COM_Outptr_ CEnumSpellingError** enumSpellingError)
{
    HRESULT hr = AtlHelper::CreateInstance(enumSpellingError);
    if (SUCCEEDED(hr))
    {
        hr = (*enumSpellingError)->Init(text, spellcheckProvider);
        if (FAILED(hr))
        {
            (*enumSpellingError)->Release();
            *enumSpellingError = nullptr;
        }
    }
    else
    {
        *enumSpellingError = nullptr;
    }

    return hr;
}

HRESULT CEnumSpellingError::Init(_In_ PCWSTR text, _In_ CSampleSpellCheckProvider* spellcheckProvider)
{
    HRESULT hr = CoTaskStringAlloc(text, &_text);

    if (SUCCEEDED(hr))
    {
        _currentTextPosition = _text;
        _spellcheckProvider = spellcheckProvider;
        _spellcheckProvider->AddRef();
    }

    return hr;
}

CEnumSpellingError::~CEnumSpellingError()
{
    _spellcheckProvider->Release();
    CoTaskMemFree(_text);
}
