////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#pragma once

// Public Headers
#include <wsdapi.h>

// Sample Headers
#include "SimpleThermostat_WSD.h"

//------------------------------------------------------------------------------
// CSimpleThermostatService
//      Implements the ISimpleThermostat_WSD Interface. This is to be used by
//      WSD device host.
//------------------------------------------------------------------------------
class CSimpleThermostatService : public ISimpleThermostat_WSD
{

private:

    ~CSimpleThermostatService();

public:

    CSimpleThermostatService();

    //
    // ISimpleThermostat_WSD Implementation
    //
    STDMETHODIMP GetCurrentTemp(
        __deref_out LONG* plTempOut
        );

    STDMETHODIMP GetDesiredTemp(
        __deref_out LONG* plTempOut
        );

    STDMETHODIMP SetDesiredTemp(
        LONG lTemp
        );

    //
    // IUnknown Implementation
    //
    STDMETHODIMP QueryInterface(
        __in REFIID riid, 
        __deref_out_opt void** ppvObject
        );
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:
    LONG m_cRef;
    LONG m_lCurrentTemp;
    LONG m_lDesiredTemp;

}; // CSimpleThermostat

