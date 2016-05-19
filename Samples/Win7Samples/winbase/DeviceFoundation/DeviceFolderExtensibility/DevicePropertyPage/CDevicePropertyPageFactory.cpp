////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// Public Headers
#include <windows.h>

// Sample Headers
#include "common.h"
#include "CDevicePropertyPageFactory.h"
#include "CDevicePropertyPage.h"

//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::CDevicePropertyPageFactory (Constructor)
//------------------------------------------------------------------------------
CDevicePropertyPageFactory::CDevicePropertyPageFactory():
    m_cRef(1)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::~CDevicePropertyPageFactory (Destructor)
//------------------------------------------------------------------------------
CDevicePropertyPageFactory::~CDevicePropertyPageFactory()
{
    DllDecLockCount();
}


//
// IClassFactory
//

//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::CreateInstance
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPageFactory::CreateInstance(
    __in_opt IUnknown* pUnkOuter,
    __in REFIID riid,
    __deref_out void** ppvObject
    )
{
    HRESULT hr  = E_FAIL;
    CDevicePropertyPage* pCDevicePropertyPage = NULL;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    if( NULL != pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    pCDevicePropertyPage = new CDevicePropertyPage();
    if( NULL == pCDevicePropertyPage )
    {
        return E_OUTOFMEMORY;
    }
	
    hr = pCDevicePropertyPage->QueryInterface( riid, ppvObject );
    pCDevicePropertyPage->Release();

    return hr;
}// CDevicePropertyPageFactory::CreateInstance


//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::LockServer
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPageFactory::LockServer(
    BOOL bLock
    )
{
    if( TRUE == bLock )
    {
        DllIncLockCount();
    }
    else
    {
        DllDecLockCount();
    }

    return S_OK;
}// CDevicePropertyPageFactory::LockServer


//
// IUnknown
//

//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::QueryInterface
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPageFactory::QueryInterface(
    __in REFIID riid, 
    __deref_out void** ppvObject
    )
{
    HRESULT hr = S_OK;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;

    if( __uuidof(IClassFactory) == riid )
    {
        *ppvObject = static_cast<IClassFactory*>(this);
        AddRef();
    }
    else if( __uuidof(IUnknown) == riid )
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}// CDevicePropertyPageFactory::QueryInterface


//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::AddRef
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDevicePropertyPageFactory::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CDevicePropertyPageFactory::AddRef


//------------------------------------------------------------------------------
// CDevicePropertyPageFactory::Release
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDevicePropertyPageFactory::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CDevicePropertyPageFactory::Release

