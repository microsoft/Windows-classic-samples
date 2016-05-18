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
#include <unknwn.h>

//------------------------------------------------------------------------------
// CSimpleThermostatProviderFactory
//      Factory class for CSimpleThermostatServiceProvider
//------------------------------------------------------------------------------
class CSimpleThermostatProviderFactory :
    public IClassFactory
{
private:

    ~CSimpleThermostatProviderFactory();

public:

    CSimpleThermostatProviderFactory();

    //
    // IClassFactory Implementation
    //
    STDMETHODIMP CreateInstance(
        __in_opt IUnknown* pUnkOuter,
        __in REFIID riid,
        __deref_out_opt void** ppvObject
        );

    STDMETHODIMP LockServer(
        BOOL bLock
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
        
};// CSimpleThermostatProviderFactory

