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
//      ClassFactory.cpp
//
//  Abstract:
//      Implementation of class factory class and helper functions.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "MyDeviceHandlerCollection.h"
#include "MyDeviceContextMenu.h"
#include "MyDevicePropertySheet.h"

// Define the GUIDs used by this component.
#include <InitGuid.h>
#include "Guids.h"

//////////////////////////////////////////////////////////////////////////////
// class CClassFactory
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the class factory.
//
//  Parameters:
//      clsid           - CLSID of the object being requested
//      riid            - Interface ID to get.
//      ppv             - Interface pointer returned to caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CClassFactory_CreateInstance(
    __in           REFCLSID      rclsid,
    __in           REFIID        riid,
    __deref_out    void        **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_OUTOFMEMORY;

    CClassFactory *pcf = new CClassFactory(rclsid);
    if (pcf != NULL)
    {
        hr = pcf->QueryInterface(riid, ppv);
        pcf->Release();
    }

    return hr;

} //*** CClassFactory_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Constructor.
//
//----------------------------------------------------------------------------
CClassFactory::CClassFactory(__in REFCLSID rclsid) : _cRef(1), _clsid(rclsid)
{
    DllAddRef();

} //*** CClassFactory::CClassFactory

//----------------------------------------------------------------------------
//
//  Description:
//      Destructor.
//
//----------------------------------------------------------------------------
CClassFactory::~CClassFactory()
{
    DllRelease();

} //*** CClassFactory::~CClassFactory

//----------------------------------------------------------------------------
// IUnknown (CClassFactory)
//----------------------------------------------------------------------------

STDMETHODIMP CClassFactory::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CClassFactory, IClassFactory),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CClassFactory::CreateInstance

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CClassFactory::Release

//----------------------------------------------------------------------------
// IClassFactory (CClassFactory)
//----------------------------------------------------------------------------

STDMETHODIMP CClassFactory::CreateInstance(__in_opt LPUNKNOWN pUnkOuter, __in REFIID riid, __deref_out void **ppv)
{
    *ppv = NULL;

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    IUnknown *punk = NULL;
    if (_clsid == CLSID_MyDeviceHandlerCollection)
    {
        hr = CMyDeviceHandlerCollection_CreateInstance(pUnkOuter, &punk);
    }
    else if (_clsid == CLSID_MyDeviceContextMenuExt)
    {
        hr = CMyDeviceContextMenu_CreateInstance(pUnkOuter, &punk);
    }
    else if (_clsid == CLSID_MyDeviceHandlerPropertySheetExt)
    {
        hr = CMyDevicePropertySheet_CreateInstance(pUnkOuter, &punk);
    }

    if (SUCCEEDED(hr))
    {
        hr = punk->QueryInterface(riid, ppv);
        punk->Release();
    }

    return hr;

} //*** CClassFactory::CreateInstance

//----------------------------------------------------------------------------

STDMETHODIMP CClassFactory::LockServer(__in BOOL fLock)
{
    if (fLock == TRUE)
    {
       DllAddRef();
    }
    else
    {
       DllRelease();
    }
    return S_OK;

} //*** CClassFactory::LockServer
