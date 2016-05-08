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
#include "CSimpleThermostatProviderFactory.h"
#include "CSimpleThermostatServiceProvider.h"
#include "common.h"

//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::CSimpleThermostatProviderFactory (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatProviderFactory::CSimpleThermostatProviderFactory():
    m_cRef(1)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::~CSimpleThermostatProviderFactory (Destructor)
//------------------------------------------------------------------------------
CSimpleThermostatProviderFactory::~CSimpleThermostatProviderFactory()
{
    DllDecLockCount();
}


//
// IClassFactory
//

//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::CreateInstance
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatProviderFactory::CreateInstance(
    IUnknown* pUnkOuter,
    REFIID riid,
    void** ppvObject
    )
{
    HRESULT hr  = E_FAIL;
    CSimpleThermostatServiceProvider* pCSimpleThermostatServiceProvider = NULL;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    if( NULL != pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    pCSimpleThermostatServiceProvider = new CSimpleThermostatServiceProvider();
    if( NULL == pCSimpleThermostatServiceProvider )
    {
        return E_OUTOFMEMORY;
    }
	
    hr = pCSimpleThermostatServiceProvider->QueryInterface( riid, ppvObject );
    pCSimpleThermostatServiceProvider->Release();

    return hr;
}// CSimpleThermostatProviderFactory::CreateInstance


//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::LockServer
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatProviderFactory::LockServer(
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
}// CSimpleThermostatProviderFactory::LockServer


//
// IUnknown
//

//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::QueryInterface
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatProviderFactory::QueryInterface(
    REFIID riid, 
    void** ppvObject
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
}// CSimpleThermostatProviderFactory::QueryInterface


//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::AddRef
//------------------------------------------------------------------------------
ULONG CSimpleThermostatProviderFactory::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CSimpleThermostatProviderFactory::AddRef


//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory::Release
//------------------------------------------------------------------------------
ULONG CSimpleThermostatProviderFactory::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CSimpleThermostatProviderFactory::Release