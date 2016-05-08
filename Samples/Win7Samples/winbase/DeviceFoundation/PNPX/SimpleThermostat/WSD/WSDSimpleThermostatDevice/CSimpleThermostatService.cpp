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

#define RAND100() (LONG)( (double)100 * ( (double)rand() / (double)RAND_MAX ) )

//------------------------------------------------------------------------------
// CSimpleThermostatService::CSimpleThermostatService (Constructor)
//------------------------------------------------------------------------------
CSimpleThermostatService::CSimpleThermostatService() :
    m_cRef(1),
    m_lDesiredTemp(0)
{
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
}


//
// ISimpleThermostat_WSD
//

//------------------------------------------------------------------------------
// CSimpleThermostatService::GetCurrentTemp
//      Gets the current temperature
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::GetCurrentTemp(
    LONG* plTempOut
    )
{
    if( NULL == plTempOut )
    {
        return E_INVALIDARG;
    }

    *plTempOut = m_lCurrentTemp;

    return S_OK;
}// CSimpleThermostatService::GetCurrentTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::GetDesiredTemp
//      Gets the desired temperature set by the user
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::GetDesiredTemp(
    LONG* plTempOut
    )
{
    if( NULL == plTempOut )
    {
        return E_INVALIDARG;
    }

    *plTempOut = m_lDesiredTemp;

    return S_OK;
}// CSimpleThermostatService::GetDesiredTemp


//------------------------------------------------------------------------------
// CSimpleThermostatService::SetDesiredTemp
//      Sets the desired temperature
//------------------------------------------------------------------------------
HRESULT CSimpleThermostatService::SetDesiredTemp(
    LONG lTemp
    )
{
    m_lDesiredTemp = lTemp;

    return S_OK;
}// CSimpleThermostatService::SetDesiredTemp


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

    if( __uuidof(ISimpleThermostat_WSD) == riid )
    {
        *ppvObject = static_cast<ISimpleThermostat_WSD*>(this);
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
    LONG lRef = InterlockedDecrement( &m_cRef );

    if( 0 == lRef )
    {
        delete this;
    }
    return lRef;
}// CSimpleThermostatService::Release
