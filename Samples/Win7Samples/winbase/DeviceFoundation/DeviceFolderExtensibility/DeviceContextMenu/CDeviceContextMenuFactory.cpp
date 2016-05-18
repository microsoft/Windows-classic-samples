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
#include "CDeviceContextMenuFactory.h"
#include "CDeviceContextMenu.h"

//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::CDeviceContextMenuFactory (Constructor)
//------------------------------------------------------------------------------
CDeviceContextMenuFactory::CDeviceContextMenuFactory():
    m_cRef(1)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::~CDeviceContextMenuFactory (Destructor)
//------------------------------------------------------------------------------
CDeviceContextMenuFactory::~CDeviceContextMenuFactory()
{
    DllDecLockCount();
}


//
// IClassFactory
//

//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::CreateInstance
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenuFactory::CreateInstance(
    __in_opt IUnknown* pUnkOuter,
    __in REFIID riid,
    __deref_out void** ppvObject
    )
{
    HRESULT hr  = E_FAIL;
    CDeviceContextMenu* pCDeviceContextMenu = NULL;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    if( NULL != pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    pCDeviceContextMenu = new CDeviceContextMenu();
    if( NULL == pCDeviceContextMenu )
    {
        return E_OUTOFMEMORY;
    }
	
    hr = pCDeviceContextMenu->QueryInterface( riid, ppvObject );
    pCDeviceContextMenu->Release();

    return hr;
}// CDeviceContextMenuFactory::CreateInstance


//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::LockServer
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenuFactory::LockServer(
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
}// CDeviceContextMenuFactory::LockServer


//
// IUnknown
//

//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::QueryInterface
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenuFactory::QueryInterface(
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
}// CDeviceContextMenuFactory::QueryInterface


//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::AddRef
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDeviceContextMenuFactory::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CDeviceContextMenuFactory::AddRef


//------------------------------------------------------------------------------
// CDeviceContextMenuFactory::Release
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDeviceContextMenuFactory::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CDeviceContextMenuFactory::Release

