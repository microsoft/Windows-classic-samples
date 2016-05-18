// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "HelloWorldContent.h"

CHelloWorldContent::CHelloWorldContent() :
    m_nRef(0)
{
    AddRef();
}

CHelloWorldContent::~CHelloWorldContent()
{
}

//
// IUnknown methods
//
HRESULT CHelloWorldContent::QueryInterface(
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
            
ULONG CHelloWorldContent::AddRef()
{
    return ::InterlockedIncrement(&m_nRef);
}
            
ULONG CHelloWorldContent::Release()
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
STDMETHODIMP CHelloWorldContent::GetContent( 
    ISideShowCapabilities* /*pICapabilities*/,
    DWORD *pdwSize,
    BYTE **ppbData
    )
{
    HRESULT hr = S_OK;

    //
    // For now, just use static content to send to the device.
    // The string needs to be UTF-8.
    //
    char* szXML = "<body><content id=\"1\" title=\"Windows SideShow Hello World\"><txt align=\"c\" wrap=\"0\">Hello Windows SideShow World</txt></content></body>";

    //
    // We need to CoTaskMemAlloc memory to return to the platform.
    // Make sure the size includes the terminating NULL character of the string!
    //
    *pdwSize = (DWORD)strlen(szXML) + 1;
    *ppbData = (BYTE*)::CoTaskMemAlloc(*pdwSize);
    if (NULL != *ppbData)
    {
        //
        // Use the safe string copy function to ensure
        // we don't overwrite the bounds of the allocated
        // buffer.
        //
        hr = StringCchCopyA((LPSTR)*ppbData, *pdwSize, szXML);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

        
STDMETHODIMP CHelloWorldContent::get_ContentId( 
    PCONTENT_ID pcontentId
    )
{
    if (NULL != pcontentId)
    {
        //
        // CONTENT_ID_HOME is the main content page which
        // is displayed when the user enters an application
        // on the device, so this is the content we
        // want to say "Hello World!"
        //
        *pcontentId = CONTENT_ID_HOME;
    }
    return S_OK;
}

        
STDMETHODIMP CHelloWorldContent::get_DifferentiateContent(
    BOOL *pfDifferentiateContent
    )
{
    if (NULL != pfDifferentiateContent)
    {
        //
        // FALSE = we do not differentiate content for different display types
        //
        *pfDifferentiateContent = FALSE;
    }
    return S_OK;
}

