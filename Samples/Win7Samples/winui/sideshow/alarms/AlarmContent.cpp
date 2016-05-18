// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "AlarmContent.h"

CAlarmContent::CAlarmContent() :
    m_nRef(0)
{
    AddRef();
}

CAlarmContent::~CAlarmContent()
{
}

//
// IUnknown methods
//
HRESULT CAlarmContent::QueryInterface(
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
            
ULONG CAlarmContent::AddRef()
{
    return ::InterlockedIncrement(&m_nRef);
}
            
ULONG CAlarmContent::Release()
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
HRESULT CAlarmContent::GetContent( 
    ISideShowCapabilities* /*pICapabilities*/,
    DWORD* pdwSize,
    BYTE** ppbData)
{
    HRESULT hr = S_OK;
    
    if (NULL == pdwSize || NULL == ppbData)
    {
        return E_INVALIDARG;
    }

    //
    // For now, just use static content to send to the device.
    // The string needs to be UTF-8.
    //
    char* szXML = "<body><content id=\"1\" title=\"Alarm Clock Sample\"><txt align=\"c\" wrap=\"1\"><br/>Alarms can be set from the Alarm Clock Sample application on the Windows Desktop.</txt></content></body>";

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

        
HRESULT CAlarmContent::get_ContentId( 
    PCONTENT_ID pcontentId)
{
    if (NULL != pcontentId)
    {
        //
        // CONTENT_ID_HOME is the main content page which
        // is displayed when the user enters an application
        // on the device.
        //
        *pcontentId = CONTENT_ID_HOME;
    }
    return S_OK;
}

        
HRESULT CAlarmContent::get_DifferentiateContent( 
    BOOL* pfDifferentiateContent)
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

