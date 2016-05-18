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
#include <upnphost.h>

// Sample Headers
#include "SimpleThermostatDevice.h"

//------------------------------------------------------------------------------
// CSimpleThermostatService
//      Implements the ISimpleThermostat_UPnPService interface and the
//      IUPnPEventSource interface (to be a UPnP device service). An object of
//      this class will be used by CSimpleThermostatDevice. This is what clients
//      will actually talk to when using the device's functionality.
//------------------------------------------------------------------------------
class CSimpleThermostatService :
    public ISimpleThermostat_UPnPService,
    public IUPnPEventSource
{
private:
    
    ~CSimpleThermostatService();

public:

    CSimpleThermostatService();

    HRESULT InitStdDispatch();

    //
    // ISimpleThermostat_UPnPService
    //
    STDMETHODIMP get_currentTemp(
        __deref_out LONG* plTemp
        );

    STDMETHODIMP get_desiredTemp(
        __deref_out LONG* plTemp
        );

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
    // IUPnPEventSource
    //
    STDMETHODIMP Advise(
        __in IUPnPEventSink* pesSubscriber
        );

    STDMETHODIMP Unadvise(
        __in IUPnPEventSink* pesSubscriber
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
    LONG            m_lCurrentTemp;
    LONG            m_lDesiredTemp;
    ITypeInfo*      m_pTypeInfo;
    IUnknown*       m_pUnkStdDispatch;
        
};// CSimpleThermostatService

