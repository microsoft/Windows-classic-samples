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
#include "CSimpleThermostatDeviceFactory.h"
#include "CSimplethermostatDevice.h"
#include "common.h"

//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::CSimpleThermostatDeviceFactory (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatDeviceFactory::CSimpleThermostatDeviceFactory():
    m_cRef(1)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::~CSimpleThermostatDeviceFactory (Destructor)
//------------------------------------------------------------------------------
CSimpleThermostatDeviceFactory::~CSimpleThermostatDeviceFactory()
{
    DllDecLockCount();
}


//
// IClassFactory
//

//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::CreateInstance
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDeviceFactory::CreateInstance(
    IUnknown* pUnkOuter,
    REFIID riid,
    void** ppvObject
    )
{
    HRESULT                     hr                          = E_FAIL;
    CSimpleThermostatDevice*    pCSimpleThermostatDevice    = NULL;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    if( NULL != pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    pCSimpleThermostatDevice = new CSimpleThermostatDevice();
    if( NULL == pCSimpleThermostatDevice )
    {
        return E_OUTOFMEMORY;
    }
	
    hr = pCSimpleThermostatDevice->QueryInterface( riid, ppvObject );
    pCSimpleThermostatDevice->Release();

    return hr;
}// CSimpleThermostatDeviceFactory::CreateInstance


//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::LockServer
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDeviceFactory::LockServer(
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
}// CSimpleThermostatDeviceFactory::LockServer


//
// IUnknown
//

//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::QueryInterface
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDeviceFactory::QueryInterface(
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
}// CSimpleThermostatDeviceFactory::QueryInterface


//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::AddRef
//------------------------------------------------------------------------------
ULONG CSimpleThermostatDeviceFactory::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CSimpleThermostatDeviceFactory::AddRef


//------------------------------------------------------------------------------
// CSimpleThermostatDeviceFactory::Release
//------------------------------------------------------------------------------
ULONG CSimpleThermostatDeviceFactory::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CSimpleThermostatDeviceFactory::Release