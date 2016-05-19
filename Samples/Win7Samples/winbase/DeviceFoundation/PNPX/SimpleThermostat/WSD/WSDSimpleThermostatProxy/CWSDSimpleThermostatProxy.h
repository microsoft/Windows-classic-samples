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
#include <FunctionDiscovery.h>

// Sample Headers
#include "SimpleThermostat.h"
#include "SimpleThermostat_WSD.h"

//
// Function Declarations
//
STDMETHODIMP CreateCWSDSimpleThermostatProxy(
    __in IPropertyStore* pPropertyStore,
    __deref_out ISimpleThermostat** ppSimpleThermostat
    );

//------------------------------------------------------------------------------
// CWSDSimpleThermostatProxy
//      This proxy object implements the real ISimpleThermostat interface and
//      knows how to talk to the ISimpleThermostat_WSD interface.
//      ISimpleThermostat_WSD is the WSD protocol specific version of the 
//      ISimpleThermostat interface.
//------------------------------------------------------------------------------
class CWSDSimpleThermostatProxy :
    public ISimpleThermostat
{
private:

    ~CWSDSimpleThermostatProxy();

public:

    CWSDSimpleThermostatProxy( __in ISimpleThermostat_WSD* pSimpleThermostat_WSD );

    //
    // ISimpleThermostat Implementation
    //
    STDMETHODIMP GetCurrentTemp(
        __deref_out LONG* plTemp
        );

    STDMETHODIMP GetDesiredTemp(
        __deref_out LONG* plTemp
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

    LONG                    m_cRef;
    ISimpleThermostat_WSD*  m_pSimpleThermostat_WSD;
        
};// CWSDSimpleThermostatProxy

