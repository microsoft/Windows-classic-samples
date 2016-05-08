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
// CDevicePropertyPageFactory
//
//      Factory class for CDevicePropertyPage
//------------------------------------------------------------------------------
class CDevicePropertyPageFactory :
    public IClassFactory
{
public:

    CDevicePropertyPageFactory();

    //
    // IClassFactory
    //
    IFACEMETHODIMP CreateInstance(
        __in_opt IUnknown* pUnkOuter,
        __in REFIID riid,
        __deref_out void** ppvObject
        );

    IFACEMETHODIMP LockServer(
        BOOL bLock
        );

    //
    // IUnknown
    //
    IFACEMETHODIMP QueryInterface(
        __in REFIID riid, 
        __deref_out void** ppvObject
        );
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

private:

    ~CDevicePropertyPageFactory();

    LONG m_cRef;
        
};// CDevicePropertyPageFactory

