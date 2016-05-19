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
#include "CSimpleThermostatService.h"

//------------------------------------------------------------------------------
// CSimpleThermostatDevice
//      This class implements the UPnP device control object which it to be
//      registered with UPnPHost as a device.
//------------------------------------------------------------------------------
class CSimpleThermostatDevice :
    public IUPnPDeviceControl
{
private:

    ~CSimpleThermostatDevice();

public:

    CSimpleThermostatDevice();

    //
    // IUPnPDeviceControl
    //
    STDMETHODIMP Initialize(
        __in BSTR bstrXMLDesc,
        __in BSTR bstrDeviceIdentifier,
        __in BSTR bstrInitString
        );
    STDMETHODIMP GetServiceObject(
        __in BSTR bstrUDN,
        __in BSTR bstrServiceId,
        __deref_out IDispatch** ppdispService
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
    CSimpleThermostatService* m_pCSimpleThermostatService;
        
};// CSimpleThermostatDevice

