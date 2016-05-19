//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      ClassFactory.h
//
//  Abstract:
//      Include file for class factory class.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CClassFactory_CreateInstance(
    __in            REFCLSID      rclsid,
    __in            REFIID        riid,
    __deref_out     void        **ppv);

//////////////////////////////////////////////////////////////////////////////
//
// Class Factory
//
//////////////////////////////////////////////////////////////////////////////
class CClassFactory : public IClassFactory
{

public:
    CClassFactory(__in REFCLSID rclsid);
    ~CClassFactory();

    // IUnknown members
    IFACEMETHODIMP           QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG)   AddRef() { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG)   Release();

    // IClassFactory members
    IFACEMETHODIMP           CreateInstance(__in_opt IUnknown* pUnkOuter, __in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP           LockServer(__in BOOL fLock);

private:
    /////////////////////
    // Member Variables
    /////////////////////
    LONG   _cRef;
    CLSID  _clsid;
};
