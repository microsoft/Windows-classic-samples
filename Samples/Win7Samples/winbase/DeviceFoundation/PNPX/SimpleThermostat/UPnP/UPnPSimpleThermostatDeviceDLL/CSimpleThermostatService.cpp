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
#include <time.h>

// Sample Headers
#include "CSimpleThermostatService.h"
#include "common.h"

#define RAND100() (LONG)( (double)100 * ( (double)rand() / (double)RAND_MAX ) )

//------------------------------------------------------------------------------
// CSimpleThermostatService::CSimpleThermostatService (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatService::CSimpleThermostatService():
    m_cRef(1),
    m_lDesiredTemp(0),
    m_pTypeInfo(NULL),
    m_pUnkStdDispatch(NULL)
{
    DllIncLockCount();

    //
    // Prep the random number generater
    //
    srand( (unsigned)time( NULL ) );
    rand();

    //
    // Since this is a virtual thermostat, a random number is generated
    // for the current temperature.
    //
    m_lCurrentTemp = RAND100();
}


//------------------------------------------------------------------------------
// CSimpleThermostatService::~CSimpleThermostatService (Destructor)
//------------------------------------------------------------------------------
CSimpleThermostatService::~CSimpleThermostatService()
{
    if( NULL != m_pUnkStdDispatch )
    {
        m_pUnkStdDispatch->Release();
        m_pUnkStdDispatch = NULL;
    }

    if( NULL != m_pTypeInfo )
    {
        m_pTypeInfo->Release();
        m_pTypeInfo = NULL;
    }

    DllDecLockCount();
}


//------------------------------------------------------------------------------
// CSimpleThermostatService::InitStdDispatch
//      Creates the standard dispatch object that will be used when IDispatch
//      is requested from this object. The UPnP device object will call this
//      function immediately after instantiation of this object.
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::InitStdDispatch()
{
    HRESULT     hr          = S_OK;
    ITypeLib*   pTypeLib    = NULL;

    hr = LoadRegTypeLib(
        LIBID_SimpleThermostatDeviceLib,
        1,
        0,
        LANG_NEUTRAL,
        &pTypeLib
        );

    if( S_OK == hr )
    {
        hr = pTypeLib->GetTypeInfoOfGuid(
            __uuidof(ISimpleThermostat_UPnPService),
            &m_pTypeInfo
            );
        pTypeLib->Release();
    }

    if( S_OK == hr )
    {
        hr = CreateStdDispatch(
            reinterpret_cast<IUnknown*>(this),
            this,
            m_pTypeInfo,
            &m_pUnkStdDispatch
            );
    }

    return hr;
}// CSimpleThermostatService::InitStdDispatch


//
// ISimpleThermostat_UPnPService
//

//------------------------------------------------------------------------------
// CSimpleThermostatService::get_currentTemp
//------------------------------------------------------------------------------
STDMETHODIMP CSimpleThermostatService::get_currentTemp(
    LONG* plTemp
    )
{
    if( NULL == plTemp )
    {
        return E_INVALIDARG;
    }

    *plTemp = m_lCurrentTemp;

    return S_OK;
}// CSimpleThermostatService::get_currentTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::get_desiredTemp
//------------------------------------------------------------------------------
STDMETHODIMP CSimpleThermostatService::get_desiredTemp(
    LONG* plTemp
    )
{
    if( NULL == plTemp )
    {
        return E_INVALIDARG;
    }

    *plTemp = m_lDesiredTemp;

    return S_OK;
}// CSimpleThermostatService::get_desiredTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::GetCurrentTemp
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::GetCurrentTemp(
    LONG* plTempOut
    )
{
    //
    // Since this function does the same thing as get_currentTemp, just call
    // get_currentTemp.
    //
    return get_currentTemp( plTempOut );
}// CSimpleThermostatService::GetCurrentTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::GetDesiredTemp
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::GetDesiredTemp(
    LONG* plTempOut
    )
{
    //
    // Since this function does the same thing as get_desiredTemp, just call
    // get_desiredTemp.
    //
    return get_desiredTemp( plTempOut );
}// CSimpleThermostatService::GetDesiredTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::SetDesiredTemp
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::SetDesiredTemp(
    LONG lTemp
    )
{
    m_lDesiredTemp = lTemp;

    return S_OK;
}// CSimpleThermostatService::SetDesiredTemp


//
// IUPnPEventSource
//

//------------------------------------------------------------------------------
// CSimpleThermostatService::Advise
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::Advise(
    IUPnPEventSink *pesSubscriber
    )
{
    return S_OK;
}// CSimpleThermostatService::Advise


//------------------------------------------------------------------------------
// CSimpleThermostatService::Unadvise
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::Unadvise(
    IUPnPEventSink *pesSubscriber
    )
{
    return S_OK;
}// CSimpleThermostatService::Unadvise


//
// IUnknown
//

//------------------------------------------------------------------------------
// CSimpleThermostatService::QueryInterface
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::QueryInterface(
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

    if( __uuidof(ISimpleThermostat_UPnPService) == riid )
    {
        *ppvObject = static_cast<ISimpleThermostat_UPnPService*>(this);
        AddRef();
    }
    else if( __uuidof(IUPnPEventSource) == riid )
    {
        *ppvObject = static_cast<IUPnPEventSource*>(this);
        AddRef();
    }
    else if( __uuidof(IDispatch) == riid )
    {
        //
        // Delegate IDispatch request to the Standard Dispatch object
        //
        hr = m_pUnkStdDispatch->QueryInterface( __uuidof(IDispatch), ppvObject );
    }
    else if( __uuidof(IUnknown) == riid )
    {
        *ppvObject = static_cast<IUnknown*>(static_cast<IUPnPEventSource*>(this));
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}// CSimpleThermostatService::QueryInterface


//------------------------------------------------------------------------------
// CSimpleThermostatService::AddRef
//------------------------------------------------------------------------------
ULONG CSimpleThermostatService::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CSimpleThermostatService::AddRef


//------------------------------------------------------------------------------
// CSimpleThermostatService::Release
//------------------------------------------------------------------------------
ULONG CSimpleThermostatService::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CSimpleThermostatService::Release