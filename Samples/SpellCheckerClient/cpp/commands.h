// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// commands.h : Implementation of commands of the interactive spell checking session

#pragma once

#include "util.h"

#include <spellcheck.h>
#include <objidl.h>
#include <stdio.h>

inline HRESULT ReadSingleWord(_In_ PCWSTR buffer, const size_t maxWordSize, _Out_writes_(maxWordSize) PWSTR word)
{
    int count = swscanf_s(buffer, L"%s", word, static_cast<unsigned int>(maxWordSize));
    HRESULT hr = (1 == count) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        _Analysis_assume_nullterminated_(word);
    }

    PrintErrorIfFailed(L"ReadSingleWord", hr);
    return hr;
}

inline HRESULT ReadTwoWords(_In_ PCWSTR buffer, const size_t maxFirstSize, _Out_writes_(maxFirstSize) PWSTR first, const size_t maxSecondSize, _Out_writes_(maxSecondSize) PWSTR second)
{
    int count = swscanf_s(buffer, L"%s %s", first, static_cast<unsigned int>(maxFirstSize), second, static_cast<unsigned int>(maxSecondSize));
    HRESULT hr = (2 == count) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        _Analysis_assume_nullterminated_(first);
        _Analysis_assume_nullterminated_(second);
    }

    PrintErrorIfFailed(L"ReadTwoWords", hr);
    return hr;
}

inline HRESULT ReadInteger(_In_ PCWSTR buffer, _Out_ int* integer)
{
    int count = swscanf_s(buffer, L"%d", integer);
    HRESULT hr = (1 == count) ? S_OK : E_FAIL;
    PrintErrorIfFailed(L"ReadInteger", hr);
    return hr;
}

inline HRESULT ReadText(_In_ PCWSTR buffer, const size_t maxTextSize, _Out_writes_(maxTextSize) PWSTR text)
{
    int count = swscanf_s(buffer, L" %[^\n]", text, static_cast<unsigned int>(maxTextSize));
    HRESULT hr = (1 == count) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        _Analysis_assume_nullterminated_(text);
    }

    PrintErrorIfFailed(L"ReadText", hr);
    return hr;
}

inline HRESULT AddCommand(_In_ ISpellChecker* spellChecker, _In_ PCWSTR buffer)
{
    wchar_t word[MAX_PATH];
    HRESULT hr = ReadSingleWord(buffer, ARRAYSIZE(word), word);
    if (SUCCEEDED(hr))
    {
        hr = spellChecker->Add(word);
    }
    PrintErrorIfFailed(L"AddCommand", hr);
    return hr;
}

inline HRESULT IgnoreCommand(_In_ ISpellChecker* spellChecker, _In_ PCWSTR buffer)
{
    wchar_t word[MAX_PATH];
    HRESULT hr = ReadSingleWord(buffer, ARRAYSIZE(word), word);
    if (SUCCEEDED(hr))
    {
        hr = spellChecker->Ignore(word);
    }
    PrintErrorIfFailed(L"IgnoreCommand", hr);
    return hr;
}

inline HRESULT AutoCorrectCommand(_In_ ISpellChecker* spellChecker, _In_ PCWSTR buffer)
{
    wchar_t from[MAX_PATH];
    wchar_t to[MAX_PATH];
    HRESULT hr = ReadTwoWords(buffer, ARRAYSIZE(from), from, ARRAYSIZE(to), to);
    if (SUCCEEDED(hr))
    {
        hr = spellChecker->AutoCorrect(from, to);
    }
    PrintErrorIfFailed(L"AutoCorrectCommand", hr);
    return hr;
}

inline HRESULT CheckCommand(_In_ ISpellChecker* spellChecker, _In_ PCWSTR buffer)
{
    wchar_t text[MAX_PATH];
    IEnumSpellingError* enumSpellingError = nullptr;
    HRESULT hr = ReadText(buffer, ARRAYSIZE(text), text);
    if (SUCCEEDED(hr))
    {
        hr = spellChecker->Check(text, &enumSpellingError);
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintSpellingErrors(spellChecker, text, enumSpellingError);
        enumSpellingError->Release();
    }
    return hr;
}

inline HRESULT CheckAsYouTypeCommand(_In_ ISpellChecker* spellChecker, _In_ PCWSTR buffer)
{
    wchar_t text[MAX_PATH];
    IEnumSpellingError* enumSpellingError = nullptr;
    HRESULT hr = ReadText(buffer, ARRAYSIZE(text), text);
    if (SUCCEEDED(hr))
    {
        hr = spellChecker->ComprehensiveCheck(text, &enumSpellingError);
    }

    if (SUCCEEDED(hr))
    {
        hr = PrintSpellingErrors(spellChecker, text, enumSpellingError);
        enumSpellingError->Release();
    }
    return hr;
}
