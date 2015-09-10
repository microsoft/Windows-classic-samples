// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// sampleengine.h : Implementation of a very simple spell checking engine, that considers correctly spelled only words starting with an specific letter

#pragma once

#include <winbase.h>
#include "engineoptions.h"
#include "strsafe.h"
#include "util.h"
#include <winuser.h>
#include <winnls.h>

const wchar_t okletterset[] = {L'a', L'b', L'f'};

class SampleEngine
{
public:
    static const size_t MAX_WORDLIST_SIZE = 10;
    static const unsigned int NUM_WORDLIST_TYPES = 4;
    static const size_t MAX_WORD_SIZE = 128;

    enum WordlistType
    {
        WordlistTypeIgnore = 0,
        WordlistTypeAdd,
        WordlistTypeExclude,
        WordlistTypeAutoCorrect
    };

    enum CorrectiveAction
    {
        CorrectiveActionNone = 0,
        CorrectiveActionGetSuggestions,
        CorrectiveActionReplace,
        CorrectiveActionDelete
    };

    struct SpellingError
    {
        size_t startIndex;
        size_t errorLength;
        CorrectiveAction correctiveAction;
        wchar_t replacement[MAX_WORD_SIZE];
    };

public:
    SampleEngine(){}

    SampleEngine(_In_ PCWSTR const languageTag)
    {
        StringCchCopy(this->languageTag, ARRAYSIZE(this->languageTag), languageTag);
        InitializeOptionValuesToDefault();
    }

    void InitializeOptionValuesToDefault()
    {
        PCWSTR optionIds[OptionsStore::MAX_LANGUAGE_OPTIONS];
        size_t numIds;
        HRESULT hr = OptionsStore::GetOptionIdsForLanguage(languageTag, &numIds, optionIds);
        for (size_t i = 0; SUCCEEDED(hr) && (i < numIds); ++i)
        {
            hr = OptionsStore::GetDefaultOptionValue(optionIds[i], &optionValues[i]);
        }
    }

    HRESULT FindFirstError(_In_ PCWSTR const text, _Out_ SpellingError* result)
    {
        HRESULT hr = S_OK;
        result->correctiveAction = CorrectiveActionNone;
        result->replacement[0] = 0;

        const wchar_t* currentPosition = text;
        while (*currentPosition != L'\0')
        {
            const wchar_t* wordStart = FindFirstNonDelimiter(currentPosition);
            if (*wordStart == L'\0')
            {
                currentPosition = wordStart;
                break;
            }

            const wchar_t* wordEnd = FindFirstDelimiter(wordStart);

            result->correctiveAction = CheckWord(wordStart, wordEnd);

            if (CorrectiveActionNone == result->correctiveAction)
            {
                const wchar_t* nextWordStart = FindFirstNonDelimiter(wordEnd);
                const wchar_t* nextWordEnd = FindFirstDelimiter(nextWordStart);

                if (ShouldIgnoreRepeatedWord() && 
                    (CSTR_EQUAL == CompareStringOrdinal(wordStart, static_cast<int>(wordEnd - wordStart),
                                                        nextWordStart, static_cast<int>(nextWordEnd - nextWordStart), FALSE)))
                {
                    result->correctiveAction = CorrectiveActionDelete;
                    result->startIndex = nextWordStart - text;
                    result->errorLength = nextWordEnd - nextWordStart;
                    break;
                }
                
                currentPosition = wordEnd;
            }
            else
            {
                result->startIndex = wordStart - text;
                result->errorLength = wordEnd - wordStart;
                if (CorrectiveActionReplace == result->correctiveAction)
                {
                    hr = GetReplacement(wordStart, wordEnd, &result->replacement);
                }

                break;
            }
        }

        if (*currentPosition == L'\0')
        {
            hr = S_FALSE;
        }

        return hr;
    }
    
    HRESULT GetSuggestions(_In_ PCWSTR const word, _In_ const size_t maxSuggestions, _Out_range_(0, maxSuggestions) size_t* numSuggestions, 
                           _Out_writes_to_(maxSuggestions, *numSuggestions) wchar_t suggestionList[][MAX_WORD_SIZE])
    {
        HRESULT hr = S_OK;
        *numSuggestions = 0;

        const wchar_t okLetter = GetOkLetter();
        for (const wchar_t* p = word; SUCCEEDED(hr) && (*p != L'\0'); ++p)
        {
            if (IsCharUpper(*p))
            {
                *numSuggestions = 0;
                break;
            }

            if (*numSuggestions < maxSuggestions)
            {
                hr = StringCchCopy(suggestionList[*numSuggestions], MAX_WORD_SIZE, word);
                if (SUCCEEDED(hr))
                {
                    size_t index = p - word;
                    suggestionList[*numSuggestions][index] = okLetter;
                    ++(*numSuggestions);
                }
            }
        }

        return hr;
    }

    HRESULT ClearWordlist(_In_ const unsigned int wordlistType)
    {
        HRESULT hr = (wordlistType >= NUM_WORDLIST_TYPES) ? E_INVALIDARG : S_OK;
        if (SUCCEEDED(hr))
        {
            numWords[wordlistType] = 0;
        }

        return hr;
    }

    HRESULT AddWordToWordlist(_In_ const unsigned int wordlistType, _In_ PCWSTR const word)
    {
        HRESULT hr = StringCchCopy(wordlists[wordlistType][numWords[wordlistType]], MAX_WORD_SIZE, word);

        if (SUCCEEDED(hr))
        {
            ++numWords[wordlistType];
        }

        return hr;
    }

    HRESULT GetLanguageTag(_In_ const unsigned long maxOutput, _Out_writes_(maxOutput) PWSTR languageTag)
    {
        return StringCchCopy(languageTag, maxOutput, this->languageTag);
    }

    HRESULT GetOptionValue(_In_ PCWSTR const optionId, _Out_ unsigned char* optionValue)
    {
        const int optionIndex = OptionsStore::GetOptionIndexInLanguage(optionId);
        HRESULT hr = (optionIndex < 0) ? E_INVALIDARG : S_OK;
        if (SUCCEEDED(hr))
        {
            *optionValue = optionValues[optionIndex];
        }

        return hr;
    }

    HRESULT SetOptionValue(_In_ PCWSTR const optionId, _In_ const unsigned char optionValue)
    {
        int optionIndex = OptionsStore::GetOptionIndexInLanguage(optionId);
        HRESULT hr = (optionIndex < 0) ? E_INVALIDARG : S_OK;

        if (SUCCEEDED(hr))
        {
            optionValues[optionIndex] = optionValue;
        }

        return hr;
    }

private:
    const wchar_t* FindFirstNonDelimiter(_In_ PCWSTR const text)
    {
        const wchar_t* p;
        for (p = text; *p != L'\0'; ++p)
        {
            if (!IsDelimiter(*p))
            {
                break;
            }
        }

        return p;
    }

    const wchar_t* FindFirstDelimiter(_In_ PCWSTR const text)
    {
        const wchar_t* p;
        for (p = text; *p != L'\0'; ++p)
        {
            if (IsDelimiter(*p))
            {
                break;
            }
        }

        return p;
    }

    bool IsDelimiter(const wchar_t c)
    {
        return ((c == L' ') || (c == L'\n') || (c == L'\t'));
    }

    CorrectiveAction CheckWord(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end)
    {
        CorrectiveAction result = CorrectiveActionNone;
        if (begin == end)
        {
            return result;
        }

        if (IsWordInWordlist(begin, end, WordlistTypeIgnore))
        {
            return CorrectiveActionNone;
        }
        else if (IsWordInWordlist(begin, end, WordlistTypeAutoCorrect))
        {
            return CorrectiveActionReplace;
        }
        else if (IsWordInWordlist(begin, end, WordlistTypeExclude))
        {
            return CorrectiveActionDelete;
        }
        else if (IsWordInWordlist(begin, end, WordlistTypeAdd))
        {
            return CorrectiveActionNone;
        }

        const bool hasOkLetter = HasOkLetter(begin, end);
        const bool hasUpper = HasUpperChar(begin, end);
        if (hasOkLetter && !hasUpper)
        {
            result = CorrectiveActionNone;
        }
        else if (hasOkLetter && hasUpper)
        {
            result = CorrectiveActionReplace;
        }
        else if (!hasOkLetter)
        {
            result = CorrectiveActionGetSuggestions; //if there's any uppercase, the suggestion list will be empty
        }

        return result;
    }

    const bool HasOkLetter(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end)
    {
        const wchar_t okLetter = GetOkLetter();
        const wchar_t upperOkLetter = reinterpret_cast<wchar_t>(CharUpper(reinterpret_cast<LPWSTR>(okLetter)));
        for (const wchar_t* p = begin; p != end; ++p)
        {
            if ((okLetter == *p) || (upperOkLetter == *p))
            {
                return true;
            }
        }

        return false;
    }

    const wchar_t GetOkLetter()
    {
        const size_t index = (optionValues[1] <= 2) ? optionValues[1] : 2;
        return okletterset[index];
    }

    const bool HasUpperChar(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end)
    {
        for (const wchar_t* p = begin; p != end; ++p)
        {
            if (IsCharUpper(*p))
            {
                return true;
            }
        }

        return false;
    }

    bool IsWordInWordlist(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end, _In_ const WordlistType wordlistType)
    {
        return (nullptr != GetWordIfInWordlist(begin, end, wordlistType));
    }

    PCWSTR GetWordIfInWordlist(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end, _In_ const WordlistType wordlistType)
    {
        unsigned int index = static_cast<unsigned int>(wordlistType);
        int comparisonSize = static_cast<int>(end - begin);

        for (size_t i = 0; i < numWords[index]; ++i)
        {
            if (CaseInsensitiveIsEqual(wordlists[index][i], begin, comparisonSize, comparisonSize))
            {
                return wordlists[index][i];
            }
        }

        return nullptr;
    }

    HRESULT GetReplacement(_In_reads_to_ptr_(end) const wchar_t* begin, _Notnull_ const wchar_t* end, _Out_ wchar_t (*replacement)[MAX_WORD_SIZE])
    {
        PCWSTR autoCorrectPair = GetWordIfInWordlist(begin, end, WordlistTypeAutoCorrect);
        if (nullptr != autoCorrectPair)
        {
            return StringCchCopy(*replacement, MAX_WORD_SIZE, autoCorrectPair + (end - begin) + 1);
        }

        HRESULT hr = StringCchCopyN(*replacement, MAX_WORD_SIZE, begin, end - begin);
        if (SUCCEEDED(hr))
        {
            CharLowerBuff(*replacement, static_cast<DWORD>(end - begin));
        }

        return hr;
    }

    bool ShouldIgnoreRepeatedWord()
    {
        return (optionValues[0] == 0);
    }

private:
    unsigned char optionValues[OptionsStore::MAX_LANGUAGE_OPTIONS];
    wchar_t languageTag[MAX_PATH];

    wchar_t wordlists[NUM_WORDLIST_TYPES][MAX_WORDLIST_SIZE][MAX_WORD_SIZE];
    size_t numWords[NUM_WORDLIST_TYPES];
};
