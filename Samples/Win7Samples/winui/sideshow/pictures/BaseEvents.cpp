// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "BaseEvents.h"

CBaseEvents::CBaseEvents() :
    m_nRef(0),
    m_pDeviceAddEvent(NULL),
    m_pDeviceRemoveEvent(NULL),
    m_pContentMissingEvent(NULL),
    m_pApplicationEventEvent(NULL)
{
    AddRef();
}

CBaseEvents::~CBaseEvents()
{
    delete m_pDeviceAddEvent;
    delete m_pDeviceRemoveEvent;
    delete m_pContentMissingEvent;
    delete m_pApplicationEventEvent;
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
    const CONTENT_ID    contentId,
    ISideShowContent**  ppIContent
    )
{
    HRESULT hr = S_OK;

    if (NULL != m_pContentMissingEvent)
    {
        hr = m_pContentMissingEvent->Call(contentId, ppIContent);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CBaseEvents::ApplicationEvent(
    ISideShowCapabilities* pICapabilities,
    const DWORD dwEventId,
    const DWORD dwEventSize,
    const BYTE* pbEventData
    )
{
    HRESULT hr = S_OK;

    if (NULL != m_pApplicationEventEvent)
    {
        hr = m_pApplicationEventEvent->Call(pICapabilities,
                                            dwEventId,
                                            dwEventSize,
                                            pbEventData);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CBaseEvents::DeviceAdded(
    ISideShowCapabilities* pIDevice
    )
{
    HRESULT hr = S_OK;

    if (NULL != m_pDeviceAddEvent)
    {
        hr = m_pDeviceAddEvent->Call(pIDevice);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CBaseEvents::DeviceRemoved(
    ISideShowCapabilities* pIDevice
    )
{
    HRESULT hr = S_OK;

    if (NULL != m_pDeviceRemoveEvent)
    {
        hr = m_pDeviceRemoveEvent->Call(pIDevice);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

void CBaseEvents::RegisterDeviceAddEvent(CCallBack* pCallBack)
{
    m_pDeviceAddEvent = pCallBack;
}

void CBaseEvents::RegisterDeviceRemoveEvent(CCallBack* pCallBack)
{
    m_pDeviceRemoveEvent = pCallBack;
}

void CBaseEvents::RegisterContentMissingEvent(CCallBack* pCallBack)
{
    m_pContentMissingEvent = pCallBack;
}

void CBaseEvents::RegisterApplicationEvent(CCallBack* pCallBack)
{
    m_pApplicationEventEvent = pCallBack;
}
