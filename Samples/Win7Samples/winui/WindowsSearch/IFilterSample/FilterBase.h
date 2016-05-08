// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/* -----------------------------------------------------------------------------------------------------

  CFilterBase

  Base class for implementing the IFilter interface

 To Use:

 The CFilterBase class is a helper class which takes care of many of the details of
 implementing an IFilter so you can just concentrate on the IFilter's task, which is to retrieve properties
 from the contents of a document.  When you derive from the CFilterBase template you need to just overload
 two methods: OnInit() and GetNextChunkValue().

 To use this class you do the following steps:
  1. create a Com Object (via Code Wizard or other method)
  2. derive it from CFilterBase
  3. CFilterBase implements the IFilter and IInitializeWithStream interfaces, so you need to update
  your QueryInterface to reflect that.

  4. Add overloaded OnInit() and GetNextChunkValue() methods

 The OnInit() method is called when the IFilter has been initialized with it's data which is available
 to you in the form of an IStream interface pointer stored in m_pStream.  (NOTE: In Windows Search only
 IStreams are supported.  For security reasons you don't have access to the file itself.) OnInit
 is the opportunity for you to do any initialization you want to do with the actual stream of data.

 After that, the indexer will call GetNextChunkValue() over and over.  Each time you are called you
 simply call the appropriate SetXXXValue method on ChunkValue with 2 pieces of information.  The first
 parameter is the property you are setting in the form of a PKEY (include propkey.h for those definitions
 or use the PSXXXX Apis to look them up).  The 2nd parameter is the actual data of the property.

 When there are no more properties to be returned you simply return FILTER_E_END_OF_CHUNKS.

 Here is an example of GetNextChunkValue() which emits 3 hard-coded properties as a way to illustrate this.

 enum EMITSTATE { EMITSTATE_ITEMNAME, EMITSTATE_CONTENTS, EMITSTATE_ISREAD};

 HRESULT CFooFilter::GetNextChunkValue(CChunkValue chunkValue)
 {
    // we post increment the emit state so that next time through it will emit the next property
    switch (m_emitState++)
    {
    case EMITSTATE_ITEMNAME:
        return chunkValue.SetTextValue(PKEY_ItemName, L"This is the title");

    case EMITSTATE_CONTENTS:
        return chunkValue.SetTextValue(PKEY_Search_Contents, L"this is the content of the item...");

    case EMITSTATE_ISREAD:
        return chunkValue.SetBoolValue(PKEY_IsRead, false);
    }
    // no more chunks
    return FILTER_E_END_OF_CHUNKS;
 }

 If you look at the SetXXXValue methods on the CChunkValue class you will see that there is support for returning a number
 of useful types:
      * Unicode strings
      * Bool and VARIANT_BOOL
      * FILETIME
      * int, long, int64

 NOTE: CChunkValue doesn't have methods to support vector (array) properties, but if you
 return a semi-colon delimited value to a vector property it will split on the semi-colon and do the right thing.
 For Example:
      to return a set of keywords (which are definied as a vector of strings) you simply:
          chunkValue.SetTextValue(PKEY_Keywords, "dog;cat;horse")

 When your COM object is ready you register it as persistent handler for your file extension
 (See the IFilter documentation for how that is done.)

 ----------------------------------------------------------------------------------------------------*/

#pragma once

#include <strsafe.h>
#include <shlwapi.h>
#include <propkey.h>
#include <propsys.h>
#include <filter.h>
#include <filterr.h>

// This is a class which simplifies both chunk and property value pair logic
// To use, you simply create a ChunkValue class of the right kind
// Example:
//      CChunkValue chunk;
//      hr = chunk.SetBoolValue(PKEY_IsAttachment, true);
//      or
//      hr = chunk.SetFileTimeValue(PKEY_ItemDate, ftLastModified);
class CChunkValue
{
public:
    CChunkValue() : m_fIsValid(false), m_pszValue(NULL)
    {
        PropVariantInit(&m_propVariant);
        Clear();
    }

    ~CChunkValue()
    {
        Clear();
    };

    // clear the ChunkValue
    void Clear()
    {
        m_fIsValid = false;
        ZeroMemory(&m_chunk, sizeof(m_chunk));
        PropVariantClear(&m_propVariant);
        CoTaskMemFree(m_pszValue);
        m_pszValue = NULL;
    }

    // Is this propvalue valid
    BOOL IsValid()
    {
        return m_fIsValid;
    }


    // get the value as an allocated PROPVARIANT
    HRESULT GetValue(PROPVARIANT **ppPropVariant)
    {
        HRESULT hr = S_OK;
        if (ppPropVariant == NULL)
        {
            return E_INVALIDARG;
        }

        *ppPropVariant = NULL;

        PROPVARIANT *pPropVariant = static_cast<PROPVARIANT*>(CoTaskMemAlloc(sizeof(PROPVARIANT)));

        if (pPropVariant)
        {
            hr = PropVariantCopy(pPropVariant, &m_propVariant);
            if (SUCCEEDED(hr))
            {
                // detach and return this as the value
                *ppPropVariant = pPropVariant;
            }
            else
            {
                CoTaskMemFree(pPropVariant);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        return hr;
    }

    // get the string value
    PWSTR GetString()
    {
        return m_pszValue;
    };

    // copy the chunk
    HRESULT CopyChunk(STAT_CHUNK *pStatChunk)
    {
        if (pStatChunk == NULL)
        {
            return E_INVALIDARG;
        }

        *pStatChunk = m_chunk;
        return S_OK;
    }

    // get the type of chunk
    CHUNKSTATE GetChunkType()
    {
        return m_chunk.flags;
    }

    // set the property by key to a unicode string
    HRESULT SetTextValue(REFPROPERTYKEY pkey, PCWSTR pszValue, CHUNKSTATE chunkType = CHUNK_VALUE,
                         LCID locale = 0, DWORD cwcLenSource = 0, DWORD cwcStartSource = 0,
                         CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        if (pszValue == NULL)
        {
            return E_INVALIDARG;
        }

        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            size_t cch = wcslen(pszValue) + 1;
            PWSTR pszCoTaskValue = static_cast<PWSTR>(CoTaskMemAlloc(cch * sizeof(WCHAR)));
            if (pszCoTaskValue)
            {
                StringCchCopy(pszCoTaskValue, cch, pszValue);
                m_fIsValid = true;
                if (chunkType == CHUNK_VALUE)
                {
                    m_propVariant.vt = VT_LPWSTR;
                    m_propVariant.pwszVal = pszCoTaskValue;
                }
                else
                {
                    m_pszValue = pszCoTaskValue;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        return hr;
    };

    // set the property by key to a bool
    HRESULT SetBoolValue(REFPROPERTYKEY pkey, BOOL bVal, CHUNKSTATE chunkType = CHUNK_VALUE, LCID locale = 0,
                         DWORD cwcLenSource = 0, DWORD cwcStartSource = 0, CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        return SetBoolValue(pkey, bVal ? VARIANT_TRUE : VARIANT_FALSE, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
    };

    // set the property by key to a variant bool
    HRESULT SetBoolValue(REFPROPERTYKEY pkey, VARIANT_BOOL bVal, CHUNKSTATE chunkType = CHUNK_VALUE, LCID locale = 0,
                         DWORD cwcLenSource = 0, DWORD cwcStartSource = 0, CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_BOOL;
            m_propVariant.boolVal = bVal;
            m_fIsValid = true;
        }
        return hr;
    };

    // set the property by key to an int
    HRESULT SetIntValue(REFPROPERTYKEY pkey, int nVal, CHUNKSTATE chunkType = CHUNK_VALUE,
                        LCID locale = 0, DWORD cwcLenSource = 0, DWORD cwcStartSource = 0,
                        CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_I4;
            m_propVariant.lVal = nVal;
            m_fIsValid = true;
        }
        return hr;
    };

    // set the property by key to a long
    HRESULT SetLongValue(REFPROPERTYKEY pkey, long lVal, CHUNKSTATE chunkType = CHUNK_VALUE, LCID locale = 0,
                         DWORD cwcLenSource = 0, DWORD cwcStartSource = 0, CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_I4;
            m_propVariant.lVal = lVal;
            m_fIsValid = true;
        }
        return hr;
    };

    // set the property by key to a dword
    HRESULT SetDwordValue(REFPROPERTYKEY pkey, DWORD dwVal, CHUNKSTATE chunkType = CHUNK_VALUE, LCID locale = 0,
                          DWORD cwcLenSource = 0, DWORD cwcStartSource = 0, CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_UI4;
            m_propVariant.ulVal = dwVal;
            m_fIsValid = true;
        }
        return hr;
    };

    // set property by key to an int64
    HRESULT SetInt64Value(REFPROPERTYKEY pkey, __int64 nVal, CHUNKSTATE chunkType = CHUNK_VALUE, LCID locale = 0,
                          DWORD cwcLenSource = 0, DWORD cwcStartSource = 0, CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_I8;
            m_propVariant.hVal.QuadPart = nVal;
            m_fIsValid = true;
        }
        return hr;
    };

    // set Property by key to a filetime
    HRESULT SetFileTimeValue(REFPROPERTYKEY pkey, FILETIME dtVal, CHUNKSTATE chunkType = CHUNK_VALUE,
                             LCID locale = 0, DWORD cwcLenSource = 0, DWORD cwcStartSource = 0,
                             CHUNK_BREAKTYPE chunkBreakType = CHUNK_NO_BREAK)
    {
        HRESULT hr = SetChunk(pkey, chunkType, locale, cwcLenSource, cwcStartSource, chunkBreakType);
        if (SUCCEEDED(hr))
        {
            m_propVariant.vt = VT_FILETIME;
            m_propVariant.filetime = dtVal;
            m_fIsValid = true;
        }
        return hr;
    };

protected:
    // set the locale for this chunk
    HRESULT SetChunk(REFPROPERTYKEY pkey, CHUNKSTATE chunkType=CHUNK_VALUE, LCID locale=0, DWORD cwcLenSource=0, DWORD cwcStartSource=0, CHUNK_BREAKTYPE chunkBreakType=CHUNK_NO_BREAK);

    // member variables
private:
    bool m_fIsValid;
    STAT_CHUNK  m_chunk;
    PROPVARIANT m_propVariant;
    PWSTR m_pszValue;

};

// Initialize the STAT_CHUNK
inline HRESULT CChunkValue::SetChunk(REFPROPERTYKEY pkey,
                                     CHUNKSTATE chunkType/*=CHUNK_VALUE*/,
                                     LCID locale /*=0*/,
                                     DWORD cwcLenSource /*=0*/,
                                     DWORD cwcStartSource /*=0*/,
                                     CHUNK_BREAKTYPE chunkBreakType /*= CHUNK_NO_BREAK */)
{
    Clear();

    // initialize the chunk
    m_chunk.attribute.psProperty.ulKind = PRSPEC_PROPID;
    m_chunk.attribute.psProperty.propid = pkey.pid;
    m_chunk.attribute.guidPropSet = pkey.fmtid;
    m_chunk.flags = chunkType;
    m_chunk.locale = locale;
    m_chunk.cwcLenSource = cwcLenSource;
    m_chunk.cwcStartSource = cwcStartSource;
    m_chunk.breakType = chunkBreakType;

    return S_OK;
}

// base class that implements IFilter and initialization interfaces for a filter
// To use:
//  - Create a COM Object derived from CFilterBase
//  - Then add IFilter, IInitializeWithStream to your COM map
//  - Implement the methods OnInit and GetNextChunkValue
class CFilterBase : public IFilter, public IInitializeWithStream
{
public:
    // pure virtual functions for derived classes to implement

    // OnInit() is called after the IStream is valid
    virtual HRESULT OnInit() = 0;

    // When GetNextChunkValue() is called you should fill in the ChunkValue by calling SetXXXValue() with the property.
    // example:  chunkValue.SetTextValue(PKYE_ItemName,L"blah de blah");
    // return FILTER_E_END_OF_CHUNKS when there are no more chunks
    virtual HRESULT GetNextChunkValue(CChunkValue &chunkValue) = 0;

protected:
    // Service functions for derived classes
    inline DWORD GetChunkId() const { return m_dwChunkId; }

public:
    CFilterBase() : m_dwChunkId(0), m_iText(0), m_pStream(NULL)
    {
    }

    virtual ~CFilterBase()
    {
        if (m_pStream)
        {
            m_pStream->Release();
        }
    }

    // IFilter
    IFACEMETHODIMP Init(ULONG grfFlags, ULONG cAttributes, const FULLPROPSPEC *aAttributes, ULONG *pFlags);
    IFACEMETHODIMP GetChunk(STAT_CHUNK *pStat);
    IFACEMETHODIMP GetText(ULONG *pcwcBuffer, WCHAR *awcBuffer);
    IFACEMETHODIMP GetValue(PROPVARIANT **ppPropValue);
    IFACEMETHODIMP BindRegion(FILTERREGION, REFIID, void **)
    {
        return E_NOTIMPL;
    }

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStm, DWORD)
    {
        if (m_pStream)
        {
            m_pStream->Release();
        }
        m_pStream = pStm;
        m_pStream->AddRef();
        return OnInit();  // derived class inits now
    };

protected:
    IStream*                    m_pStream;         // stream of this document

private:
    DWORD                       m_dwChunkId;        // Current chunk id
    DWORD                       m_iText;            // index into ChunkValue

    CChunkValue                 m_currentChunk;     // the current chunk value
};

HRESULT CFilterBase::Init(ULONG, ULONG, const FULLPROPSPEC *, ULONG *)
{
    // Common initialization
    m_dwChunkId = 0;
    m_iText = 0;
    m_currentChunk.Clear();
    return S_OK;
}

HRESULT CFilterBase::GetChunk(STAT_CHUNK *pStat)
{
    HRESULT hr = S_OK;

    // Get the chunk from the derived class.  A return of S_FALSE indicates the chunk should be skipped and we should
    // try to get the next chunk.

    int cIterations = 0;
    hr = S_FALSE;

    while ((S_FALSE == hr) && (~cIterations & 0x100))  // Limit to 256 iterations for safety
    {
        pStat->idChunk = m_dwChunkId;
        hr = GetNextChunkValue(m_currentChunk);
        ++cIterations;
    }

    if (hr == S_OK)
    {
        if (m_currentChunk.IsValid())
        {
            // copy out the STAT_CHUNK
            m_currentChunk.CopyChunk(pStat);

            // and set the id to be the sequential chunk
            pStat->idChunk = ++m_dwChunkId;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

HRESULT CFilterBase::GetText(ULONG *pcwcBuffer, WCHAR *awcBuffer)
{
    HRESULT hr = S_OK;

    if ((pcwcBuffer == NULL) || (*pcwcBuffer == 0))
    {
        return E_INVALIDARG;
    }

    if (!m_currentChunk.IsValid())
    {
        return FILTER_E_NO_MORE_TEXT;
    }

    if (m_currentChunk.GetChunkType() != CHUNK_TEXT)
    {
        return FILTER_E_NO_TEXT;
    }

    ULONG cchTotal = static_cast<ULONG>(wcslen(m_currentChunk.GetString()));
    ULONG cchLeft = cchTotal - m_iText;
    ULONG cchToCopy = min(*pcwcBuffer - 1, cchLeft);

    if (cchToCopy > 0)
    {
        PCWSTR psz = m_currentChunk.GetString() + m_iText;

        // copy the chars
        StringCchCopyN(awcBuffer, *pcwcBuffer, psz, cchToCopy);

        // null terminate it
        awcBuffer[cchToCopy] = '\0';

        // set how much data is copied
        *pcwcBuffer = cchToCopy;

        // remember we copied it
        m_iText += cchToCopy;
        cchLeft -= cchToCopy;

        if (cchLeft == 0)
        {
            hr = FILTER_S_LAST_TEXT;
        }
    }
    else
    {
        hr = FILTER_E_NO_MORE_TEXT;
    }

    return hr;
}

HRESULT CFilterBase::GetValue(PROPVARIANT **ppPropValue)
{
    HRESULT hr = S_OK;

    // if this is not a value chunk they shouldn't be calling this
    if (m_currentChunk.GetChunkType() != CHUNK_VALUE)
    {
        return FILTER_E_NO_MORE_VALUES;
    }

    if (ppPropValue == NULL)
    {
        return E_INVALIDARG;
    }

    if (m_currentChunk.IsValid())
    {
        // return the value of this chunk as a PROPVARIANT ( they own freeing it properly )
        hr = m_currentChunk.GetValue(ppPropValue);
        m_currentChunk.Clear();
    }
    else
    {
        // we have already return the value for this chunk, go away
        hr = FILTER_E_NO_MORE_VALUES;
    }

    return hr;
}
