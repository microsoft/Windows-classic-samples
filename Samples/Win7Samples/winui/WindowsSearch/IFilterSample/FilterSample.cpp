// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <new>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <xmllite.h>
#include "FilterBase.h"

void DllAddRef();
void DllRelease();

// Filter for ".filtersample" files

class CFilterSample : public CFilterBase
{
public:
    CFilterSample() : m_cRef(1), m_iEmitState(EMITSTATE_FLAGSTATUS), m_pReader(NULL)
    {
        DllAddRef();
    }

    ~CFilterSample()
    {
        if (m_pReader)
        {
            m_pReader->Release();
        }
        DllRelease();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CFilterSample, IInitializeWithStream),
            QITABENT(CFilterSample, IFilter),
            {0, 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    virtual HRESULT OnInit();
    virtual HRESULT GetNextChunkValue(CChunkValue &chunkValue);

private:
    // helper method for XmlLite
    HRESULT _GetElementText(PCWSTR *ppszText, UINT *pcwchValue);

    long m_cRef;

    // XmlLite reader on the m_pStream
    IXmlReader *m_pReader;

    // some props we want to emit don't come from the doc.  We use this as our state
    enum EMITSTATE { EMITSTATE_FLAGSTATUS = 0, EMITSTATE_ISREAD };
    DWORD m_iEmitState;
};

HRESULT CFilterSample_CreateInstance(REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CFilterSample *pFilter = new (std::nothrow) CFilterSample();
    if (pFilter)
    {
        hr = pFilter->QueryInterface(riid, ppv);
        pFilter->Release();
    }
    return hr;
}

// This is called after the stream (m_pStream) has been setup and is ready for use
// This implementation of this filter passes that stream to the xml reader
HRESULT CFilterSample::OnInit()
{
    HRESULT hr = CreateXmlReader(IID_PPV_ARGS(&m_pReader), NULL);
    if (SUCCEEDED(hr))
    {
        hr = m_pReader->SetInput(m_pStream);
    }
    return hr;
}

// When GetNextChunkValue() is called we fill in the ChunkValue by calling SetXXXValue() with the property and value (and other parameters that you want)
// example:  chunkValue.SetTextValue(PKEY_ItemName, L"example text");
// return FILTER_E_END_OF_CHUNKS when there are no more chunks
HRESULT CFilterSample::GetNextChunkValue(CChunkValue &chunkValue)
{
    HRESULT hr = S_OK;

    chunkValue.Clear();

    // read through the stream
    XmlNodeType nodeType;
    while (S_OK == (hr = m_pReader->Read(&nodeType)))
    {
        if (XmlNodeType_Element == nodeType)
        {
            PCWSTR pszName = NULL;
            PCWSTR pszValue = NULL;

            hr = m_pReader->GetLocalName(&pszName, NULL);
            if (FAILED(hr))
            {
                return hr;
            }

            // if it is the title
            if (wcscmp(pszName, L"mytitle") == 0)
            {
                hr = _GetElementText(&pszValue, NULL);
                if (SUCCEEDED(hr))
                {
                    // return this value chunk
                    chunkValue.SetTextValue(PKEY_Title, pszValue);
                    return S_OK;
                }
            }
            // if it is the my keywords
            else if (wcscmp(pszName, L"mykeywords") == 0)
            {
                hr = _GetElementText(&pszValue, NULL);
                if (SUCCEEDED(hr))
                {
                    // Note commas or semicolons can be used as separators between multi-valued strings
                    chunkValue.SetTextValue(PKEY_Keywords, pszValue);
                    return S_OK;
                }
            }
            // if it is the my author
            else if (wcscmp(pszName, L"Author") == 0)
            {
                hr = _GetElementText(&pszValue, NULL);
                if (SUCCEEDED(hr))
                {
                    // return this value chunk
                    chunkValue.SetTextValue(PKEY_ItemAuthors, pszValue);
                    return S_OK;
                }
            }
            // if it is the my body
            else if (wcscmp(pszName, L"lastmodified") == 0)
            {
                hr = _GetElementText(&pszValue, NULL);
                if (SUCCEEDED(hr))
                {
                    // in this sample this element uses a string {e.g. "12/1/2009"}, so parse and convert to a FILETIME
                    PCWSTR pszMonth = pszValue;
                    if (pszMonth && pszMonth[0] != L'\0')
                    {
                        PCWSTR pszDay = wcschr(pszMonth + 1, L'/');
                        if (pszDay++)
                        {
                            PCWSTR pszYear = wcschr(pszDay, L'/');
                            if (pszYear++)
                            {
                                SYSTEMTIME systime = {0};
                                systime.wMonth = static_cast<WORD>(_wtoi(pszMonth));
                                systime.wDay = static_cast<WORD>(_wtoi(pszDay));
                                systime.wYear = static_cast<WORD>(_wtoi(pszYear));

                                FILETIME filetime;
                                SystemTimeToFileTime(&systime, &filetime);
                                chunkValue.SetFileTimeValue(PKEY_DateModified, filetime);
                            }
                        }
                    }
                    return S_OK;
                }
            }
            // if it is the my body
            else if (wcscmp(pszName, L"body") == 0)
            {
                hr = _GetElementText(&pszValue, NULL);
                if (SUCCEEDED(hr))
                {
                    // This is the indexable body (it is not stored or retrieved but just indexed over)
                    // we pass CHUNK_TEXT so that it is treated as a stream of text, not a flat property string.
                    chunkValue.SetTextValue(PKEY_Search_Contents, pszValue, CHUNK_TEXT);
                    return S_OK;
                }
            }
            // If we found an element of interest then the value is stored in chunkValue and we return.
            // Otherwise continue until another element is found.
        }
    }

    // Not all data from the XML document has been read but additional props can be added
    // For this we use the m_iEmitState to iterate through them, each call will go to the next one
    switch (m_iEmitState)
    {
        case EMITSTATE_FLAGSTATUS:
            // we are using this just to illustrate a numeric property
            chunkValue.SetIntValue(PKEY_FlagStatus, 1);
            m_iEmitState++;
            return S_OK;

        case EMITSTATE_ISREAD:
            // we are using this just to illustrate a bool property
            chunkValue.SetIntValue(PKEY_IsRead, true);
            m_iEmitState++;
            return S_OK;
    }

    // if we get to here we are done with this document
    return FILTER_E_END_OF_CHUNKS;
}

// utility method for getting the contents of an element
// example: <foo>test</foo> will return [test]
HRESULT CFilterSample::_GetElementText(PCWSTR *ppszText, UINT *pcwchValue)
{
    XmlNodeType nodeType;
    HRESULT hr = S_OK;
    while (S_OK == (hr = m_pReader->Read(&nodeType)))
    {
        if (nodeType == XmlNodeType_Text)
        {
            return m_pReader->GetValue(ppszText, pcwchValue);
        }
    }
    return hr;
}

