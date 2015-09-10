//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#include <ole2.h>
#include <xmllite.h>
#include <stdio.h>
#include <shlwapi.h>

#pragma warning(push)
#pragma warning(disable : 28196)
#pragma warning(disable: 6001)
#pragma warning(disable: 6054)
#pragma warning(disable: 26018)
#pragma warning(disable: 26036)
#pragma warning(disable: 26061)
#include <strsafe.h>
#pragma warning(pop)

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

class DateXmlResolver : public IXmlResolver
{
public:
    // Constructor
    DateXmlResolver()
    {
        m_cref = 1;
    }
    // Destructor
	virtual ~DateXmlResolver() {}

  // IUnknown interface
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out _Result_nullonfailure_ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if (ppvObject == NULL)
        {
            return E_INVALIDARG;
        }
        (*ppvObject) = NULL;

        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IXmlResolver))
        {
            *ppvObject = static_cast<IXmlResolver *>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return ++m_cref;
    }

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        ULONG cref;

        cref = --m_cref;
        if (cref == 0)
        {
            delete this;
        }
        return cref;
    }

  // IXmlResolver interface
    virtual HRESULT STDMETHODCALLTYPE ResolveUri(_In_opt_ LPCWSTR,
        _In_opt_ LPCWSTR ,
        _In_opt_ LPCWSTR  pwszSystemIdentifier,
        _Out_ IUnknown ** ppResolvedInput)
    {
        IStream *pstream = NULL;
        HRESULT hr;

        if (!pwszSystemIdentifier || wcscmp(pwszSystemIdentifier, L"local://current_date") != 0)
        {
            // We can only resolve local://current_date 'URI'
            // Please note, that the above string is an arbitrary value
            // the reader doesn't look into it in any way.
            // (It doesn't have to look like URI for example)

            // Other URIs are not implemented
            return E_NOTIMPL;
        }

        // Get the current date and time
        SYSTEMTIME systime;
        GetLocalTime(&systime);

        WCHAR pwch[256];
        // Prepend UTF-16 BOM -> we need the parser to recognize the input
        // We could also use CreateXmlReaderInputWithCodePage to specify the encoding
        hr = ::StringCchPrintf(pwch, 256, L"\xFEFF%d. %d. %d.", systime.wDay, systime.wMonth, systime.wYear);
        if (FAILED(hr))
        {
            return hr;
        }
        SIZE_T cwch = wcslen(pwch);

        // Allocate HGLOBAL object so that we can use CreateStreamOnHGlobal
        HGLOBAL hglobal;
        hglobal = ::GlobalAlloc(GMEM_MOVEABLE, cwch * sizeof(WCHAR));
        if (hglobal == NULL)
        {
            return E_OUTOFMEMORY;
        }

        // Lock it and copy the data into the allocated memory
        LPVOID pv = ::GlobalLock(hglobal);
        if (pv == NULL)
        {
            ::GlobalFree(hglobal);
            return E_OUTOFMEMORY;
        }
        memcpy(pv, pwch, cwch * sizeof(WCHAR));

        // And create the stream
        hr = ::CreateStreamOnHGlobal(hglobal, TRUE, &pstream);
        if (FAILED(hr))
        {
            ::GlobalFree(hglobal);
            return hr;
        }

        // And ask the stream for its IUnknown
        hr = pstream->QueryInterface(__uuidof(IUnknown), (void**)ppResolvedInput);
        if (FAILED(hr))
        {
            pstream->Release();
            return hr;
        }
        return S_OK;
    }
private:
    ULONG m_cref;
};


HRESULT WriteAttributes(IXmlReader* pReader)
{
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    HRESULT hr = pReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx", hr);
        return hr;
    }
    else
    {
        for (;;)
        {
            if (!pReader->IsDefault())
            {
                UINT cwchPrefix;
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    return hr;
                }
                if (cwchPrefix > 0)
                    wprintf(L"Attr: %s:%s=\"%s\" \n", pwszPrefix, pwszLocalName, pwszValue);
                else
                    wprintf(L"Attr: %s=\"%s\" \n", pwszLocalName, pwszValue);
            }

            if (S_OK != pReader->MoveToNextAttribute())
                break;
        }
    }
    return hr;
}

int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    HRESULT hr = S_OK;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    UINT cwchPrefix;
	DateXmlResolver* pXMLResolver = NULL; 

    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteReader.exe name-of-input-file\n");
        return 0;
    }

    #pragma warning(suppress: 28197)   //pXMLResolver will be deleted by SAFE_RELEASE(pXMLResolver) below
    pXMLResolver = new DateXmlResolver();
    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(argv[1], STGM_READ, &pFileStream)))
    {
        wprintf(L"Error creating file reader, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx", hr);
        HR(hr);
    }
   
    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Parse)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
        HR(hr);
    }

	 if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_XmlResolver, (LONG_PTR)pXMLResolver)))
    {
        wprintf(L"Error setting XmlReaderProperty_XmlResolver, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx", hr);
        HR(hr);
    }

    //read until there are no more nodes
    while (S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
        case XmlNodeType_XmlDeclaration:
            wprintf(L"XmlDeclaration\n");
            if (FAILED(hr = WriteAttributes(pReader)))
            {
                wprintf(L"Error writing attributes, error is %08.8lx", hr);
                HR(hr);
            }
            break;
        case XmlNodeType_Element:
            if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error getting prefix, error is %08.8lx", hr);
                HR(hr);
            }
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting local name, error is %08.8lx", hr);
                HR(hr);
            }
            if (cwchPrefix > 0)
                wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"Element: %s\n", pwszLocalName);

            if (FAILED(hr = WriteAttributes(pReader)))
            {
                wprintf(L"Error writing attributes, error is %08.8lx", hr);
                HR(hr);
            }

            if (pReader->IsEmptyElement() )
                wprintf(L" (empty)");
            break;
        case XmlNodeType_EndElement:
            if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error getting prefix, error is %08.8lx", hr);
                HR(hr);
            }
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting local name, error is %08.8lx", hr);
                HR(hr);
            }
            if (cwchPrefix > 0)
                wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"End Element: %s\n", pwszLocalName);
            break;
        case XmlNodeType_Text:
        case XmlNodeType_Whitespace:
            if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                HR(hr);
            }
            wprintf(L"Text: >%s<\n", pwszValue);
            break;
        case XmlNodeType_CDATA:
            if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                HR(hr);
            }
            wprintf(L"CDATA: %s\n", pwszValue);
            break;
        case XmlNodeType_ProcessingInstruction:
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting name, error is %08.8lx", hr);
                HR(hr);
            }
            if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                HR(hr);
            }
            wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue);
            break;
        case XmlNodeType_Comment:
            if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                HR(hr);
            }
            wprintf(L"Comment: %s\n", pwszValue);
            break;
        case XmlNodeType_DocumentType:
            wprintf(L"DOCTYPE is not printed\n");
            break;
        }
    }

CleanUp:
    SAFE_RELEASE(pXMLResolver);
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;
}
