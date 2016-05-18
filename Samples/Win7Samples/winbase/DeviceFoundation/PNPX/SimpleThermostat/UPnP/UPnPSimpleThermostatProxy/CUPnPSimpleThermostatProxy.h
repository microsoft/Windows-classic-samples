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
#include <windows.h>
#include <upnp.h>
#include <FunctionDiscovery.h>

// Sample Headers
#include "SimpleThermostat.h"

//
// Function Declarations
//
HRESULT CreateCUPnPSimpleThermostatProxy(
    __in IPropertyStore* pPropertyStore,
    __deref_out ISimpleThermostat** ppSimpleThermostat
    );

//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy
//      This proxy object implements the ISimpleThermostat interface and
//      knows how to talk to the device supporting the 
//      ISimpleThermostat_UPnPService interface.
//------------------------------------------------------------------------------
class CUPnPSimpleThermostatProxy :
    public ISimpleThermostat
{
private:

    ~CUPnPSimpleThermostatProxy();

public:

    CUPnPSimpleThermostatProxy( __in IUPnPDevice* pUPnPDevice );

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

    LONG            m_cRef;
    IUPnPDevice*    m_pUPnPDevice;
        
};// CUPnPSimpleThermostatProxy

