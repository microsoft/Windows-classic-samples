// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionarySearch.h"
#include "SampleIMEBaseStructure.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CDictionarySearch::CDictionarySearch(LCID locale, _In_ CFile *pFile, _In_ CStringRange *pSearchKeyCode) : CDictionaryParser(locale)
{
    _pFile = pFile;
    _pSearchKeyCode = pSearchKeyCode;
    _charIndex = 0;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CDictionarySearch::~CDictionarySearch()
{
}

//+---------------------------------------------------------------------------
//
// FindPhrase
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhrase(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, FALSE); // NO WILDCARD
}

//+---------------------------------------------------------------------------
//
// FindPhraseForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhraseForWildcard(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindConvertedStringForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindConvertedStringForWildcard(CDictionaryResult **ppdret)
{
    return FindWorker(TRUE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindWorker
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch)
{
    DWORD_PTR dwTotalBufLen = GetBufferInWCharLength();        // in char
    if (dwTotalBufLen == 0)
    {
        return FALSE;
    }

    const WCHAR *pwch = GetBufferInWChar();
    DWORD_PTR indexTrace = 0;     // in char
    *ppdret = nullptr;
    BOOL isFound = FALSE;
    DWORD_PTR bufLenOneLine = 0;

TryAgain:
    bufLenOneLine = GetOneLine(&pwch[indexTrace], dwTotalBufLen);
    if (bufLenOneLine == 0)
    {
        goto FindNextLine;
    }
    else
    {
        CParserStringRange keyword;
        DWORD_PTR bufLen = 0;
        LPWSTR pText = nullptr;

        if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword))
        {
            return FALSE;    // error
        }

        if (!isTextSearch)
        {
            // Compare Dictionary key code and input key code
            if (!isWildcardSearch)
            {
                if (CStringRange::Compare(_locale, &keyword, _pSearchKeyCode) != CSTR_EQUAL)
                {
                    if (bufLen)
                    {
                        delete [] pText;
                    }
                    goto FindNextLine;
                }
            }
            else
            {
                // Wildcard search
                if (!CStringRange::WildcardCompare(_locale, _pSearchKeyCode, &keyword))
                {
                    if (bufLen)
                    {
                        delete [] pText;
                    }
                    goto FindNextLine;
                }
            }
        }
        else
        {
            // Compare Dictionary converted string and input string
            CSampleImeArray<CParserStringRange> convertedStrings;
            if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword, &convertedStrings))
            {
                if (bufLen)
                {
                    delete [] pText;
                }
                return FALSE;
            }
            if (convertedStrings.Count() == 1)
            {
                CStringRange* pTempString = convertedStrings.GetAt(0);

                if (!isWildcardSearch)
                {
                    if (CStringRange::Compare(_locale, pTempString, _pSearchKeyCode) != CSTR_EQUAL)
                    {
                        if (bufLen)
                        {
                            delete [] pText;
                        }
                        goto FindNextLine;
                    }
                }
                else
                {
                    // Wildcard search
                    if (!CStringRange::WildcardCompare(_locale, _pSearchKeyCode, pTempString))
                    {
                        if (bufLen)
                        {
                            delete [] pText;
                        }
                        goto FindNextLine;
                    }
                }
            }
            else
            {
                if (bufLen)
                {
                    delete [] pText;
                }
                goto FindNextLine;
            }
        }

        if (bufLen)
        {
            delete [] pText;
        }

        // Prepare return's CDictionaryResult
        *ppdret = new (std::nothrow) CDictionaryResult();
        if (!*ppdret)
        {
            return FALSE;
        }

        CSampleImeArray<CParserStringRange> valueStrings;
        if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword, &valueStrings))
        {
            if (*ppdret)
            {
                delete *ppdret;
                *ppdret = nullptr;
            }
            return FALSE;
        }

        (*ppdret)->_FindKeyCode = keyword;
        (*ppdret)->_SearchKeyCode = *_pSearchKeyCode;

        for (UINT i = 0; i < valueStrings.Count(); i++)
        {
            CStringRange* findPhrase = (*ppdret)->_FindPhraseList.Append();
            if (findPhrase)
            {
                *findPhrase = *valueStrings.GetAt(i);
            }
        }

        // Seek to next line
        isFound = TRUE;
    }

FindNextLine:
    dwTotalBufLen -= bufLenOneLine;
    if (dwTotalBufLen == 0)
    {
        indexTrace += bufLenOneLine;
        _charIndex += indexTrace;

        if (!isFound && *ppdret)
        {
            delete *ppdret;
            *ppdret = nullptr;
        }
        return (isFound ? TRUE : FALSE);        // End of file
    }

    indexTrace += bufLenOneLine;
    if (pwch[indexTrace] == L'\r' || pwch[indexTrace] == L'\n' || pwch[indexTrace] == L'\0')
    {
        bufLenOneLine = 1;
        goto FindNextLine;
    }

    if (isFound)
    {
        _charIndex += indexTrace;
        return TRUE;
    }

    goto TryAgain;
}
