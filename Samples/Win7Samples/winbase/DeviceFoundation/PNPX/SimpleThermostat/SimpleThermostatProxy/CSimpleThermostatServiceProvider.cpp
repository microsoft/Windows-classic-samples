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
#include <stdio.h>

// Sample Headers
#include "CSimpleThermostatServiceProvider.h"
#include "CUPnPSimpleThermostatProxy.h"
#include "CWSDSimpleThermostatProxy.h"
#include "common.h"

//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::CSimpleThermostatServiceProvider (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatServiceProvider::CSimpleThermostatServiceProvider():
    m_cRef(1)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::~CSimpleThermostatServiceProvider (Destructor)
//------------------------------------------------------------------------------
CSimpleThermostatServiceProvider::~CSimpleThermostatServiceProvider()
{
    DllDecLockCount();
}


//
// IFunctionDiscoveryServiceProvider
//

//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::Initialize
//      This function is called by Function Discovery (FD) when QueryService is
//      called on an FD Function Instance specifying
//      CLSID_SimpleThermostatProxy. This function creates a WSD or UPnP proxy
//      object depending on what type of device the FD Function Instance
//      represents.
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatServiceProvider::Initialize( 
    IFunctionInstance* pFunInst,
    REFIID riid,
    void** ppv
    )
{
    DEVICE_PROTOCOL_TYPE    deviceType  = DEVICE_PROTOCOL_TYPE_UPNP;
    HRESULT                 hr          = S_OK;
    IPropertyStore*         pPropStore  = NULL;

    if( NULL == ppv ||
        NULL == pFunInst )
    {
        return E_INVALIDARG;
    }

    //
    // Open the property store of the function instance
    //
    if( S_OK == hr )
    {   
        wprintf( L"OpenPropertyStore on the FI..." );
        hr = pFunInst->OpenPropertyStore( STGM_READ, &pPropStore );
        wprintf( L"0x%x\n", hr );
    }

    //
    // See if this is a device type supported. This is done by checking
    // the 'Type' property inside the FI's property store.
    //
    if( S_OK == hr )
    {
        wprintf( L"GetDeviceType..." );
        hr = GetDeviceType( pPropStore, &deviceType );
        wprintf( L"0x%x\n", hr );
        if( S_FALSE == hr )
        {
            wprintf( L"Device type unsupported!" );
        }
    }

    if( S_OK == hr )
    {
        wprintf( L"Device type: %s\n",
            DEVICE_PROTOCOL_TYPE_UPNP == deviceType ? L"UPNP" : L"WSD" );
    }

    //
    // Create the appropriate proxy
    //
    if( S_OK == hr &&
        DEVICE_PROTOCOL_TYPE_UPNP == deviceType )
    {
        hr = CreateCUPnPSimpleThermostatProxy(
            pPropStore,
            reinterpret_cast<ISimpleThermostat**>(ppv)
            );
    }

    else if( S_OK == hr &&
        DEVICE_PROTOCOL_TYPE_WSD == deviceType )
    {
        hr = CreateCWSDSimpleThermostatProxy(
            pPropStore,
            reinterpret_cast<ISimpleThermostat**>(ppv)
            );
    }

    //
    // Cleanup
    //
    if( NULL != pPropStore )
    {
        pPropStore->Release();
        pPropStore = NULL;
    }

    return hr;
}// CSimpleThermostatServiceProvider::Initialize


//
// IUnknown
//

//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::QueryInterface
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatServiceProvider::QueryInterface(
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

    if( __uuidof(IFunctionDiscoveryServiceProvider) == riid )
    {
        *ppvObject = static_cast<IFunctionDiscoveryServiceProvider*>(this);
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
}// CSimpleThermostatServiceProvider::QueryInterface


//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::AddRef
//------------------------------------------------------------------------------
ULONG CSimpleThermostatServiceProvider::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CSimpleThermostatServiceProvider::AddRef


//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider::Release
//------------------------------------------------------------------------------
ULONG CSimpleThermostatServiceProvider::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CSimpleThermostatServiceProvider::Release
