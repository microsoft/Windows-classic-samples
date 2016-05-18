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
#include "CSimpleThermostatDevice.h"
#include "CSimpleThermostatService.h"
#include "common.h"

//------------------------------------------------------------------------------
// CSimpleThermostatDevice::CSimpleThermostatDevice (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatDevice::CSimpleThermostatDevice():
    m_cRef(1),
    m_pCSimpleThermostatService(NULL)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CSimpleThermostatDevice::~CSimpleThermostatDevice (Destructor)
//------------------------------------------------------------------------------
CSimpleThermostatDevice::~CSimpleThermostatDevice()
{
    if( NULL != m_pCSimpleThermostatService )
    {
        m_pCSimpleThermostatService->Release();
        m_pCSimpleThermostatService = NULL;
    }

    DllDecLockCount();
}


//
// IUPnPDeviceControl
//

//------------------------------------------------------------------------------
// CSimpleThermostatDevice::Initialize
//      This function is called by the UPnPHost when someone registers this
//      device with UPnPHost.
//      
//      For this implementation, initialize just creates the devices service
//      object and initializes it.
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDevice::Initialize(
    BSTR bstrXMLDesc,
    BSTR bstrDeviceIdentifier,
    BSTR bstrInitString
    )
{
    if( NULL == bstrXMLDesc ||
        NULL == bstrDeviceIdentifier ||
        NULL == bstrInitString )
    {
        return E_INVALIDARG;
    }

    //
    // Create the service object
    //
    m_pCSimpleThermostatService = new CSimpleThermostatService();
    if( NULL == m_pCSimpleThermostatService )
    {
        return E_OUTOFMEMORY;
    }

    //
    // Initialize the standard dispatch object within the service object
    //
    return m_pCSimpleThermostatService->InitStdDispatch();
}// CSimpleThermostatDevice::Initialize


//------------------------------------------------------------------------------
// CSimpleThermostatDevice::GetServiceObject
//      Called when a user of this device requests one of the services. Since
//      the device only has one service it just always returns the same service.
//      Typically the ServiceId should be checked to return the correct service.
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDevice::GetServiceObject(
    BSTR bstrUDN,
    BSTR bstrServiceId,
    IDispatch** ppdispService
    )
{
    if( NULL == bstrUDN ||
        NULL == bstrServiceId ||
        NULL == ppdispService )
    {
        return E_INVALIDARG;
    }

    return m_pCSimpleThermostatService->QueryInterface(
        __uuidof(IDispatch),
        reinterpret_cast<void**>(ppdispService)
        );
}// CSimpleThermostatDevice::GetServiceObject


//
// IUnknown
//

//------------------------------------------------------------------------------
// CSimpleThermostatDevice::QueryInterface
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatDevice::QueryInterface(
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

    if( __uuidof(IUPnPDeviceControl) == riid )
    {
        *ppvObject = static_cast<IUPnPDeviceControl*>(this);
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
}// CSimpleThermostatDevice::QueryInterface


//------------------------------------------------------------------------------
// CSimpleThermostatDevice::AddRef
//------------------------------------------------------------------------------
ULONG CSimpleThermostatDevice::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CSimpleThermostatDevice::AddRef


//------------------------------------------------------------------------------
// CSimpleThermostatDevice::Release
//------------------------------------------------------------------------------
ULONG CSimpleThermostatDevice::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CSimpleThermostatDevice::Release