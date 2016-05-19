// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "BaseEvents.h"

CBaseEvents::CBaseEvents() :
    m_nRef(0)
{
    AddRef();
}

CBaseEvents::~CBaseEvents()
{

}

//
// IUnknown methods
//
HRESULT CBaseEvents::QueryInterface(
    REFIID riid,
    LPVOID* ppvObject
    )
{
    if (IID_IUnknown == riid || 
        IID_ISideShowEvents == riid)
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}
            
ULONG CBaseEvents::AddRef()
{
    return ::InterlockedIncrement(&m_nRef);
}
            
ULONG CBaseEvents::Release()
{
    LONG nRef = ::InterlockedDecrement(&m_nRef);
    if (0 == nRef)
    {
        delete this;
    }

    return nRef;
}

//
// ISideShowEvents methods
//
HRESULT CBaseEvents::ContentMissing(
    const CONTENT_ID /*contentId*/,
    ISideShowContent** /*ppIContent*/)
{
    return S_FALSE;
}

HRESULT CBaseEvents::ApplicationEvent(
    ISideShowCapabilities* /*pICapabilities*/,
    const DWORD /*dwEventId*/,
    const DWORD /*dwEventSize*/,
    const BYTE* /*pbEventData*/)
{
    return S_FALSE;
}

HRESULT CBaseEvents::DeviceAdded(
    ISideShowCapabilities* /*pIDevice*/)
{
    return S_FALSE;
}

HRESULT CBaseEvents::DeviceRemoved(
    ISideShowCapabilities* /*pIDevice*/)
{
    return S_FALSE;
}
