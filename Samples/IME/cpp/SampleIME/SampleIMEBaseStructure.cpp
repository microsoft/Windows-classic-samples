// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Globals.h"

//---------------------------------------------------------------------
//
// CLSIDToString
//
//---------------------------------------------------------------------

const BYTE GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
};

const WCHAR HexDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR *pCLSIDString)
{
    WCHAR* pTemp = pCLSIDString;
    const BYTE* pBytes = (const BYTE *) &refGUID;

    DWORD j = 0;
    pTemp[j++] = L'{';
    for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++)
    {
        if (GuidSymbols[i] == '-')
        {
            pTemp[j++] = L'-';
        }
        else
        {
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0xF0) >> 4 ];
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0x0F) ];
        }
    }

    pTemp[j++] = L'}';
    pTemp[j] = L'\0';

    return TRUE;
}

//---------------------------------------------------------------------
//
// SkipWhiteSpace
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && IsSpace(locale, *pwszBuffer) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// FindChar
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && (*pwszBuffer != wch) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// IsSpace
//
//---------------------------------------------------------------------

BOOL IsSpace(LCID locale, WCHAR wch)
{
    WORD wCharType = 0;

    GetStringTypeEx(locale, CT_CTYPE1, &wch, 1, &wCharType);
    return (wCharType & C1_SPACE);
}

CStringRange::CStringRange()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

CStringRange::~CStringRange() 
{ 
}

const WCHAR *CStringRange::Get() const
{
    return _pStringBuf;
}

const DWORD_PTR CStringRange::GetLength() const
{
    return _stringBufLen;
}

void CStringRange::Clear()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

void CStringRange::Set(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = pwch;
}

void CStringRange::Set(CStringRange &sr)
{
    *this = sr;
}

CStringRange& CStringRange::operator =(const CStringRange& sr)
{
    _stringBufLen = sr._stringBufLen;
    _pStringBuf = sr._pStringBuf;
    return *this;
}

void CStringRange::CharNext(_Inout_ CStringRange* pCharNext)
{
    if (!_stringBufLen)
    {
        pCharNext->_stringBufLen = 0;
        pCharNext->_pStringBuf = nullptr;
    }
    else
    {
        pCharNext->_stringBufLen = _stringBufLen;
        pCharNext->_pStringBuf = _pStringBuf;

        while (pCharNext->_stringBufLen)
        {
            BOOL isSurrogate = (IS_HIGH_SURROGATE(*pCharNext->_pStringBuf) || IS_LOW_SURROGATE(*pCharNext->_pStringBuf));
            pCharNext->_stringBufLen--;
            pCharNext->_pStringBuf++;
            if (!isSurrogate)
            {
                break;
            }
        }
    }
}

int CStringRange::Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2)
{
    return CompareString(locale, 
        NORM_IGNORECASE, 
        pString1->Get(), 
        (DWORD)pString1->GetLength(), 
        pString2->Get(), 
        (DWORD)pString2->GetLength());
}

BOOL CStringRange::WildcardCompare(LCID locale, _In_ CStringRange* stringWithWildcard, _In_ CStringRange* targetString)
{
    if (stringWithWildcard->GetLength() == 0)
    {
        return targetString->GetLength() == 0 ? TRUE : FALSE;
    }

    CStringRange stringWithWildcard_next;
    CStringRange targetString_next;
    stringWithWildcard->CharNext(&stringWithWildcard_next);
    targetString->CharNext(&targetString_next);

    if (*stringWithWildcard->Get() == L'*')
    {
        return WildcardCompare(locale, &stringWithWildcard_next, targetString) || ((targetString->GetLength() != 0) && WildcardCompare(locale, stringWithWildcard, &targetString_next));
    }
    if (*stringWithWildcard->Get() == L'?')
    {
        return ((targetString->GetLength() != 0) && WildcardCompare(locale, &stringWithWildcard_next, &targetString_next));
    }

    BOOL isSurrogate1 = (IS_HIGH_SURROGATE(*stringWithWildcard->Get()) || IS_LOW_SURROGATE(*stringWithWildcard->Get()));
    BOOL isSurrogate2 = (IS_HIGH_SURROGATE(*targetString->Get()) || IS_LOW_SURROGATE(*targetString->Get()));

    return ((CompareString(locale,
        NORM_IGNORECASE,
        stringWithWildcard->Get(),
        (isSurrogate1 ? 2 : 1),
        targetString->Get(),
        (isSurrogate2 ? 2 : 1)) == CSTR_EQUAL) && WildcardCompare(locale, &stringWithWildcard_next, &targetString_next));
}

CCandidateRange::CCandidateRange(void)
{
}


CCandidateRange::~CCandidateRange(void)
{
}


BOOL CCandidateRange::IsRange(UINT vKey)
{
    DWORD value = vKey - L'0';

    for (UINT i = 0; i < _CandidateListIndexRange.Count(); i++)
    {
        if (value == *_CandidateListIndexRange.GetAt(i))
        {
            return TRUE;
        }
        else if ((VK_NUMPAD0 <= vKey) && (vKey <= VK_NUMPAD9))
        {
            if ((vKey-VK_NUMPAD0) == *_CandidateListIndexRange.GetAt(i))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int CCandidateRange::GetIndex(UINT vKey)
{
    DWORD value = vKey - L'0';

    for (UINT i = 0; i < _CandidateListIndexRange.Count(); i++)
    {
        if (value == *_CandidateListIndexRange.GetAt(i))
        {
            return i;
        }
        else if ((VK_NUMPAD0 <= vKey) && (vKey <= VK_NUMPAD9))
        {
            if ((vKey-VK_NUMPAD0) == *_CandidateListIndexRange.GetAt(i))
            {
                return i;
            }
        }
    }
    return -1;
}

CPunctuationPair::CPunctuationPair()
{
    _punctuation._Code = 0;
    _punctuation._Punctuation = 0;
    _pairPunctuation = 0;
    _isPairToggle = FALSE;
}

CPunctuationPair::CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair)
{
    _punctuation._Code = code;
    _punctuation._Punctuation = punctuation;
    _pairPunctuation = pair;
    _isPairToggle = FALSE;
}

CPunctuationNestPair::CPunctuationNestPair()
{
    _punctuation_begin._Code = 0;
    _punctuation_begin._Punctuation = 0;
    _pairPunctuation_begin = 0;

    _punctuation_end._Code = 0;
    _punctuation_end._Punctuation = 0;
    _pairPunctuation_end = 0;

    _nestCount = 0;
}

CPunctuationNestPair::CPunctuationNestPair(WCHAR codeBegin, WCHAR punctuationBegin, WCHAR pairBegin,
    WCHAR codeEnd,   WCHAR punctuationEnd,   WCHAR pairEnd)
{
	pairEnd;punctuationEnd;
    _punctuation_begin._Code = codeBegin;
    _punctuation_begin._Punctuation = punctuationBegin;
    _pairPunctuation_begin = pairBegin;

    _punctuation_end._Code = codeEnd;
    _punctuation_end._Punctuation = punctuationBegin;
    _pairPunctuation_end = pairBegin;

    _nestCount  = 0;
}