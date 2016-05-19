// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "BaseContent.h"

CBaseContent::CBaseContent() :
    m_nRef(0),
    m_contentID(CONTENT_ID_HOME)
{
    AddRef();
}

CBaseContent::~CBaseContent()
{
}


//
// IUnknown methods
//
HRESULT CBaseContent::QueryInterface(
    REFIID riid,
    LPVOID* ppvObject
    )
{
    if (IID_IUnknown == riid || 
        IID_ISideShowContent == riid)
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}
            
ULONG CBaseContent::AddRef()
{
    return ::InterlockedIncrement(&m_nRef);
}
            
ULONG CBaseContent::Release()
{
    LONG nRef = ::InterlockedDecrement(&m_nRef);
    if (0 == nRef)
    {
        delete this;
    }

    return nRef;
}


//
// ISideShowContent methods
//
HRESULT CBaseContent::GetContent(
    ISideShowCapabilities* /*pICapabilities*/,
    DWORD *pdwSize,
    BYTE **ppbData)
{
    HRESULT hr = S_OK;
    DWORD   dwSize = 0;
    BYTE*   pbData = NULL;

    //
    // Call the subclass to retrieve the
    // content and size to add.
    //
    LoadContent(&dwSize, &pbData);

    if (NULL != pbData)
    {
        //
        // COM requires memory passed out to be alloc'ed by
        // CoTaskMemAlloc.  Allocate enough to hold the
        // data returned by the subclass and then copy
        // the data into the new buffer.
        //
        BYTE* pbBuf = (BYTE*)::CoTaskMemAlloc(dwSize);
        if (NULL != pbBuf)
        {
            memcpy(pbBuf, pbData, dwSize);

            //
            // Assuming all went well, return the
            // information to the platform.
            //
            *pdwSize = dwSize;
            *ppbData = pbBuf;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        //
        // Call the subclass to free the data;
        // pbData is no longer valid after this call
        //
        FreeContent(&pbData);
        pbData = NULL;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}


HRESULT CBaseContent::get_ContentId(
    PCONTENT_ID pcontentId)
{
    if (NULL != pcontentId)
    {
        //
        // Return the CONTENT_ID associated with
        // this instance of the content object.
        // The member variable m_contentID should
        // be overwritten by subclasses according
        // to the content.
        //
        *pcontentId = m_contentID;
    }
    return S_OK;
}


HRESULT CBaseContent::get_DifferentiateContent(
    BOOL *pfDifferentiateContent)
{
    if (NULL != pfDifferentiateContent)
    {
        //
        // FALSE = we do not differentiate content for different display types
        //
        *pfDifferentiateContent = TRUE;
    }
    return S_OK;
}


LPSTR CBaseContent::AllocTaskUtf8String(LPCWSTR lpszString)
{
    if (lpszString == NULL)
    {
        return NULL;
    }

    UINT nBytes = (UINT)((wcslen(lpszString)+1)*2);
    LPSTR lpszResult = (LPSTR)::CoTaskMemAlloc(nBytes);
    if (lpszResult != NULL)
    {
        int nRet = WideCharToMultiByte(CP_UTF8, 0, lpszString, -1, lpszResult, nBytes, NULL, NULL);
        if (nRet == 0)
        {
            ::CoTaskMemFree(lpszResult);
            lpszResult = NULL;
        }
    }
    return lpszResult;
}
