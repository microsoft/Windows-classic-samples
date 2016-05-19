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
#include <FunctionDiscoveryProvider.h>

//------------------------------------------------------------------------------
// CSimpleThermostatServiceProvider
//      This class is used by Function Discovery (FD) to create the correct
//      proxy object when a client asks for CLSID_SimpleThermostatProxy from an
//      FD Function Instance supporting ISimpleThermostat.
//------------------------------------------------------------------------------
class CSimpleThermostatServiceProvider :
    public IFunctionDiscoveryServiceProvider
{
private:

    ~CSimpleThermostatServiceProvider();

public:

    CSimpleThermostatServiceProvider();

    //
    // IFunctionDiscoveryServiceProvider Implementation
    //
    STDMETHODIMP Initialize( 
        __in IFunctionInstance *pIFunctionInstance,
        __in REFIID riid,
        __deref_out_opt void **ppvObject
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
        
};// CSimpleThermostatServiceProvider

