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

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

class Malloc : public IMalloc
{
public:
    // Constructor
    Malloc()
    {
        m_cref = 1;
        m_cbTotalAllocated = 0;
        // Set total max to 1 MB
        m_cbAllocatedMaximum = 1024 * 1024;
        // Set the single buffer max to 10 KB
        m_cbSingleBufferMaximum = 10 * 1024;
    }
    // Destructor
    virtual ~Malloc() {}

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out _Result_nullonfailure_ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if (ppvObject == NULL)
        {
            return E_INVALIDARG;
        }
        (*ppvObject) = NULL;

        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IMalloc))
        {
            *ppvObject = static_cast<IMalloc *>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return ++m_cref;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG cref;

        cref = --m_cref;
        if (cref == 0)
        {
            delete this;
        }
        return cref;
    }

    // IMalloc
    void __RPC_FAR *STDMETHODCALLTYPE Alloc(/* [in] */_In_ SIZE_T cb)
    {
        if ((m_cbSingleBufferMaximum > 0) && (cb > m_cbSingleBufferMaximum))
        {
            // The requested buffer exceeded the allowed maximum.
            return NULL;
        }
        m_cbTotalAllocated += cb;
        if ((m_cbAllocatedMaximum > 0) && (m_cbTotalAllocated > m_cbAllocatedMaximum))
        {
            // Total number of allocated bytes exceeded the allowed maximum.
            return NULL;
        }
        return ::HeapAlloc(GetProcessHeap(), 0, cb);
    }

    void __RPC_FAR *STDMETHODCALLTYPE Realloc(/* [in] */ _In_opt_ void __RPC_FAR *,/* [in] */ _In_ SIZE_T )
    {
        return NULL;
    }

    void STDMETHODCALLTYPE Free(/* [in] */ _In_opt_ void* pv)
    {
        ::HeapFree(GetProcessHeap(), 0, pv);
    }

    SIZE_T STDMETHODCALLTYPE GetSize(/* [in] */ _In_opt_ void*)
    {
        return 0;
    }

    int STDMETHODCALLTYPE DidAlloc(_In_opt_ void*)
    {
        return -1;
    }

    void STDMETHODCALLTYPE HeapMinimize() {}

    // Sets the maximum number of bytes allowed to be allocated.
    // If 0 -> infinite 
    void SetAllocatedMaximum(SIZE_T cbMaximum) { m_cbAllocatedMaximum = cbMaximum; }
    // Sets the maximum number of bytes allowed for every single allocation.
    // If 0 -> infinite
    void SetSingleBufferMaximum(SIZE_T cbMaximum) 
    { 
        m_cbSingleBufferMaximum = cbMaximum; 
    }

private:
    // Reference counter
    LONG m_cref;

    // The  total number of bytes allocated.
    SIZE_T m_cbTotalAllocated;
    // The  maximum number of bytes allowed to be allocated.
    // If 0 -> infinite
    SIZE_T m_cbAllocatedMaximum;
    // The  maximum number of bytes allowed for every single allocation.
    // If 0 -> infinite
    SIZE_T m_cbSingleBufferMaximum;
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
    Malloc *pMalloc = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix = NULL;
    const WCHAR* pwszLocalName = NULL;
    const WCHAR* pwszValue = NULL;
    UINT cwchPrefix;

    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteReader.exe name-of-input-file\n");
        return 0;
    }

    // The following will limit allocations by default to
    // total max to  1 MB:
    // m_cbAllocatedMaximum = 1024 * 1024;
    // the single buffer max to 10 KB
    // m_cbSingleBufferMaximum = 10 * 1024;
    #pragma warning(suppress: 28197) //pMalloc will be deleted by SAFE_RELEASE(pMalloc)
    pMalloc = new Malloc();

    // Open a read-only input stream.
    if (FAILED(hr = SHCreateStreamOnFile(argv[1], STGM_READ, &pFileStream)))
    {
        wprintf(L"Error creating file reader, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, pMalloc)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx ", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx", hr);
        HR(hr);
    }

    // Read until there are no more nodes.
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
    SAFE_RELEASE(pMalloc);
    SAFE_RELEASE(pReader);
    SAFE_RELEASE(pFileStream);
    return hr;
}
