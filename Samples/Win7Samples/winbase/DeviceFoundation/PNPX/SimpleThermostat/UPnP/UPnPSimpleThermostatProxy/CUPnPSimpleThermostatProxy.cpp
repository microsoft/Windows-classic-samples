////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// Public Headers
#include <windows.h>
#include <stdio.h>
#include <strsafe.h>

// Sample Headers
#include "CUPnPSimpleThermostatProxy.h"

//
// Globals
//
const WCHAR GETCURRENTTEMP_FUNC[] = L"GetCurrentTemp";
const WCHAR GETDESIREDTEMP_FUNC[] = L"GetDesiredTemp";
const WCHAR SETDESIREDTEMP_FUNC[] = L"SetDesiredTemp";
const WCHAR DEVICE_SERVICE[]      = L"urn:upnp-org:serviceId:ISimpleThermostatUPnPService";


//------------------------------------------------------------------------------
// CreateCUPnPSimpleThermostatProxy
//      This is essentially the factory method for the
//      CUPnPSimpleThermostatProxy class. It takes a property store that's
//      expected to have come from an FD Function Instance.
//------------------------------------------------------------------------------
HRESULT CreateCUPnPSimpleThermostatProxy(
    IPropertyStore* pPropertyStore,
    ISimpleThermostat** ppSimpleThermostat
    )
{
    BSTR                        bstrDeviceID                = NULL;
    BSTR                        bstrXMLDocURL               = NULL;
    HRESULT                     hr                          = S_OK;
    IUPnPDescriptionDocument*   pUPnPDescription            = NULL;
    IUPnPDevice*                pUPnPDevice                 = NULL;
    PROPVARIANT                 pvDeviceID                  = {0};
    PROPVARIANT                 pvXMLDocURL                 = {0};
    CUPnPSimpleThermostatProxy* pCUPnPSimpleThermostatProxy = NULL;

    if( NULL == pPropertyStore ||
        NULL == ppSimpleThermostat )
    {
        return E_INVALIDARG;
    }

    wprintf( L"GetValue of PKEY_PNPX_GlobalIdentity (device host id)..." );
    hr = pPropertyStore->GetValue( PKEY_PNPX_GlobalIdentity, &pvDeviceID );
    wprintf( L"0x%x\n", hr );

    if( S_OK == hr )
    {
        wprintf( L"Device Host ID: %s\n", pvDeviceID.pwszVal );
    }

    if( S_OK == hr )
    {
        wprintf( L"GetValue of PKEY_Device_LocationInfo (Description Doc URL)..." );
        hr = pPropertyStore->GetValue( PKEY_Device_LocationInfo, &pvXMLDocURL );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        wprintf( L"Description Doc URL: %s\n", pvXMLDocURL.pwszVal );
    }

    //
    // Create the description document object that'll be use to obtain the
    // IUPnPDevice object
    //
    if( S_OK == hr )
    {
        wprintf( L"CoCreate the UPnPDescriptionDocument object..." );
        hr = CoCreateInstance(
            CLSID_UPnPDescriptionDocument,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IUPnPDescriptionDocument,
            reinterpret_cast<void**>(&pUPnPDescription)
            );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        bstrXMLDocURL = SysAllocString( pvXMLDocURL.pwszVal );
        if( NULL == bstrXMLDocURL )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    //
    // Load the description document
    // 
    if( S_OK == hr )
    {
        wprintf( L"Load the device's description document..." );
        hr = pUPnPDescription->Load( bstrXMLDocURL );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        bstrDeviceID = SysAllocString( pvDeviceID.pwszVal );
        if( NULL == bstrDeviceID )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    //
    // Get the upnp device object
    //
    if( S_OK == hr )
    {
        wprintf( L"Get the UPnPDevice object from UPnP..." );
        hr = pUPnPDescription->DeviceByUDN( bstrDeviceID, &pUPnPDevice );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Create the CUPnPSimpleThermostatProxy object and hand it
    // the UPnPDevice object.
    //
    if( S_OK == hr )
    {
        wprintf( L"Creating the CUPnPSimpleThermostatProxy object..." );
        pCUPnPSimpleThermostatProxy = new CUPnPSimpleThermostatProxy( pUPnPDevice );
        if( NULL == pCUPnPSimpleThermostatProxy )
        {
            hr = E_OUTOFMEMORY;
        }
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        *ppSimpleThermostat = static_cast<ISimpleThermostat*>(pCUPnPSimpleThermostatProxy);
    }

    //
    // Cleanup
    //
    PropVariantClear( &pvDeviceID );
    PropVariantClear( &pvXMLDocURL );

    if( NULL != pUPnPDevice )
    {
        pUPnPDevice->Release();
        pUPnPDevice = NULL;
    }

    if( NULL != bstrDeviceID )
    {
        SysFreeString( bstrDeviceID );
        bstrDeviceID = NULL;
    }

    if( NULL != bstrXMLDocURL )
    {
        SysFreeString( bstrXMLDocURL );
        bstrDeviceID = NULL;
    }

    if( NULL != pUPnPDescription )
    {
        pUPnPDescription->Release();
        pUPnPDescription = NULL;
    }

    return hr;
}// CreateCUPnPSimpleThermostatProxy


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::CUPnPSimpleThermostatProxy (Constructor)
//------------------------------------------------------------------------------
CUPnPSimpleThermostatProxy::CUPnPSimpleThermostatProxy( IUPnPDevice* pUPnPDevice ):
    m_cRef(1)
{
    pUPnPDevice->AddRef();
    m_pUPnPDevice = pUPnPDevice;
}


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::~CUPnPSimpleThermostatProxy (Destructor)
//------------------------------------------------------------------------------
CUPnPSimpleThermostatProxy::~CUPnPSimpleThermostatProxy()
{
    if( NULL != m_pUPnPDevice )
    {
        m_pUPnPDevice->Release();
        m_pUPnPDevice = NULL;
    }
}


//
// ISimpleThermostat
//

//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::GetCurrentTemp
//------------------------------------------------------------------------------
HRESULT CUPnPSimpleThermostatProxy::GetCurrentTemp(
    LONG* plTemp
    )
{
    BSTR            bstrFunc        = NULL;
    BSTR            bstrServiceID   = NULL;
    HRESULT         hr              = S_OK;
    SAFEARRAYBOUND  inArgsBound[1];
    IUPnPService*   pService        = NULL;
    IUPnPServices*  pServices       = NULL;
    SAFEARRAY*      psaInArgs       = NULL;
    SAFEARRAY*      psaOutArgs      = NULL;
    LONG            rgIndices[1]    = {0};
    VARIANT         varInArgs       = {0};
    VARIANT         varOutArgs      = {0};
    VARIANT         varRet          = {0};
    VARIANT         varTemp         = {0};

    
    if( NULL == plTemp )
    {
        return E_INVALIDARG;
    }

    bstrFunc = SysAllocString( GETCURRENTTEMP_FUNC );
    bstrServiceID = SysAllocString( DEVICE_SERVICE );
    if( NULL == bstrFunc ||
        NULL == bstrServiceID )
    {
        hr = E_OUTOFMEMORY;
    }

    VariantInit( &varInArgs );
    VariantInit( &varOutArgs );
    VariantInit( &varRet );
    VariantInit( &varTemp );

    //
    // Grab all the services on the device
    //
    if( S_OK == hr )
    {
        hr = m_pUPnPDevice->get_Services( &pServices );
    }

    //
    // Get the ISimpleThermostatUPnPService service
    //
    if( S_OK == hr )
    {
        hr = pServices->get_Item( bstrServiceID, &pService );
    }

    //
    // Build the arguments to pass to the function
    //
    if( S_OK == hr )
    {
        inArgsBound[0].lLbound = 0;
        inArgsBound[0].cElements = 0;

        psaInArgs = SafeArrayCreate( VT_VARIANT, 1, inArgsBound );
        if( NULL == psaInArgs )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            varInArgs.vt = VT_VARIANT | VT_ARRAY;
            V_ARRAY( &varInArgs ) = psaInArgs;
        }
    }

    //
    // Invoke the function
    //
    if( S_OK == hr )
    {
        hr = pService->InvokeAction( bstrFunc, varInArgs, &varOutArgs, &varRet );
        if( S_OK != hr )
        {
            LONG lStatus;
            pService->get_LastTransportStatus( &lStatus );
            wprintf( L"InvokeAction failed...%d...", lStatus );
        }
    }

    if( S_OK == hr )
    {
        psaOutArgs = V_ARRAY( &varOutArgs );
        rgIndices[0] = 0;

        hr = SafeArrayGetElement(
            psaOutArgs,
            rgIndices,
            (void*)&varTemp
            );
    }

    if( S_OK == hr )
    {
        *plTemp = varTemp.ulVal;
    }

    //
    // Cleanup
    //
    VariantClear( &varRet );
    VariantClear( &varTemp );

    if( NULL != psaInArgs )
    {
        SafeArrayDestroy( psaInArgs );
        psaInArgs = NULL;
    }

    if( NULL != psaOutArgs )
    {
        SafeArrayDestroy( psaOutArgs );
        psaInArgs = NULL;
    }

    if( NULL != pService )
    {
        pService->Release();
        pService = NULL;
    }

    if( NULL != pServices )
    {
        pServices->Release();
        pServices = NULL;
    }

    if( NULL != bstrServiceID )
    {
        SysFreeString( bstrServiceID );
        bstrServiceID = NULL;
    }

    if( NULL != bstrFunc )
    {
        SysFreeString( bstrFunc );
        bstrFunc = NULL;
    }

    return hr;
}// CUPnPSimpleThermostatProxy::GetCurrentTemp


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::GetDesiredTemp
//------------------------------------------------------------------------------
HRESULT CUPnPSimpleThermostatProxy::GetDesiredTemp(
    LONG* plTemp
    )
{
    BSTR            bstrFunc        = NULL;
    BSTR            bstrServiceID   = NULL;
    HRESULT         hr              = S_OK;
    SAFEARRAYBOUND  inArgsBound[1];
    IUPnPService*   pService        = NULL;
    IUPnPServices*  pServices       = NULL;
    SAFEARRAY*      psaInArgs       = NULL;
    SAFEARRAY*      psaOutArgs      = NULL;
    LONG            rgIndices[1]    = {0};
    VARIANT         varInArgs       = {0};
    VARIANT         varOutArgs      = {0};
    VARIANT         varRet          = {0};
    VARIANT         varTemp         = {0};


    if( NULL == plTemp )
    {
        return E_INVALIDARG;
    }
    
    bstrFunc = SysAllocString( GETDESIREDTEMP_FUNC );
    bstrServiceID = SysAllocString( DEVICE_SERVICE );
    if( NULL == bstrFunc ||
        NULL == bstrServiceID )
    {
        hr = E_OUTOFMEMORY;
    }

    VariantInit( &varInArgs );
    VariantInit( &varOutArgs );
    VariantInit( &varRet );
    VariantInit( &varTemp );

    //
    // Grab all the services on the device
    //
    if( S_OK == hr )
    {
        hr = m_pUPnPDevice->get_Services( &pServices );
    }

    //
    // Get the ISimpleThermostatUPnPService service
    //
    if( S_OK == hr )
    {
        hr = pServices->get_Item( bstrServiceID, &pService );
    }

    //
    // Build the arguments to pass to the function
    //
    if( S_OK == hr )
    {
        inArgsBound[0].lLbound = 0;
        inArgsBound[0].cElements = 0;

        psaInArgs = SafeArrayCreate( VT_VARIANT, 1, inArgsBound );
        if( NULL == psaInArgs )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            varInArgs.vt = VT_VARIANT | VT_ARRAY;
            V_ARRAY( &varInArgs ) = psaInArgs;
        }
    }

    //
    // Invoke the function
    //
    if( S_OK == hr )
    {
        hr = pService->InvokeAction( bstrFunc, varInArgs, &varOutArgs, &varRet );
    }

    if( S_OK == hr )
    {
        psaOutArgs = V_ARRAY( &varOutArgs );
        rgIndices[0] = 0;

        hr = SafeArrayGetElement(
            psaOutArgs,
            rgIndices,
            (void*)&varTemp
            );
    }

    if( S_OK == hr )
    {
        *plTemp = varTemp.ulVal;
    }

    //
    // Cleanup
    //
    VariantClear( &varRet );
    VariantClear( &varTemp );

    if( NULL != psaInArgs )
    {
        SafeArrayDestroy( psaInArgs );
        psaInArgs = NULL;
    }

    if( NULL != psaOutArgs )
    {
        SafeArrayDestroy( psaOutArgs );
        psaInArgs = NULL;
    }

    if( NULL != pService )
    {
        pService->Release();
        pService = NULL;
    }

    if( NULL != pServices )
    {
        pServices->Release();
        pServices = NULL;
    }

    if( NULL != bstrServiceID )
    {
        SysFreeString( bstrServiceID );
        bstrServiceID = NULL;
    }

    if( NULL != bstrFunc )
    {
        SysFreeString( bstrFunc );
        bstrFunc = NULL;
    }

    return hr;
}// CUPnPSimpleThermostatProxy::GetDesiredTemp


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::SetDesiredTemp
//------------------------------------------------------------------------------
HRESULT CUPnPSimpleThermostatProxy::SetDesiredTemp(
    LONG lTemp
    )
{
    BSTR            bstrFunc        = NULL;
    BSTR            bstrServiceID   = NULL;
    HRESULT         hr              = S_OK;
    SAFEARRAYBOUND  inArgsBound[1];
    IUPnPService*   pService        = NULL;
    IUPnPServices*  pServices       = NULL;
    SAFEARRAY*      psaInArgs       = NULL;
    LONG            rgIndices[1]    = {0};
    VARIANT         varInArgs       = {0};
    VARIANT         varOutArgs      = {0};
    VARIANT         varRet          = {0};
    VARIANT         varTemp         = {0};

    
    bstrFunc = SysAllocString( SETDESIREDTEMP_FUNC );
    bstrServiceID = SysAllocString( DEVICE_SERVICE );
    if( NULL == bstrFunc ||
        NULL == bstrServiceID )
    {
        hr = E_OUTOFMEMORY;
    }

    VariantInit( &varInArgs );
    VariantInit( &varOutArgs );
    VariantInit( &varRet );
    VariantInit( &varTemp );

    //
    // Grab all the services on the device
    //
    if( S_OK == hr )
    {
        hr = m_pUPnPDevice->get_Services( &pServices );
    }

    //
    // Get the ISimpleThermostatUPnPService service
    //
    if( S_OK == hr )
    {
        hr = pServices->get_Item( bstrServiceID, &pService );
    }

    //
    // Build the arguments to pass to the function
    //
    if( S_OK == hr )
    {
        inArgsBound[0].lLbound = 0;
        inArgsBound[0].cElements = 1;

        psaInArgs = SafeArrayCreate( VT_VARIANT, 1, inArgsBound );
        if( NULL == psaInArgs )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        varTemp.vt = VT_I4;
        varTemp.ulVal = lTemp;
        hr = SafeArrayPutElement( psaInArgs, rgIndices, (void*)&varTemp );
    }

    if( S_OK == hr )
    {
        varInArgs.vt = VT_VARIANT | VT_ARRAY;
        V_ARRAY( &varInArgs ) = psaInArgs;
    }

    //
    // Invoke the function
    //
    if( S_OK == hr )
    {
        hr = pService->InvokeAction( bstrFunc, varInArgs, &varOutArgs, &varRet );
    }

    //
    // Cleanup
    //
    VariantClear( &varOutArgs );
    VariantClear( &varRet );
    VariantClear( &varTemp );

    if( NULL != psaInArgs )
    {
        SafeArrayDestroy( psaInArgs );
        psaInArgs = NULL;
    }

    if( NULL != pService )
    {
        pService->Release();
        pService = NULL;
    }

    if( NULL != pServices )
    {
        pServices->Release();
        pServices = NULL;
    }

    if( NULL != bstrServiceID )
    {
        SysFreeString( bstrServiceID );
        bstrServiceID = NULL;
    }

    if( NULL != bstrFunc )
    {
        SysFreeString( bstrFunc );
        bstrFunc = NULL;
    }

    return hr;
}// CUPnPSimpleThermostatProxy::SetDesiredTemp


//
// IUnknown
//

//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::QueryInterface
//------------------------------------------------------------------------------
HRESULT CUPnPSimpleThermostatProxy::QueryInterface(
    REFIID riid, 
    void** ppvObject
    )
{
    HRESULT hr = S_OK;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;

    if( __uuidof(ISimpleThermostat) == riid )
    {
        *ppvObject = static_cast<ISimpleThermostat*>(this);
        AddRef();
    }
    else if( __uuidof(IUnknown) == riid )
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}// CUPnPSimpleThermostatProxy::QueryInterface


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::AddRef
//------------------------------------------------------------------------------
ULONG CUPnPSimpleThermostatProxy::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CUPnPSimpleThermostatProxy::AddRef


//------------------------------------------------------------------------------
// CUPnPSimpleThermostatProxy::Release
//------------------------------------------------------------------------------
ULONG CUPnPSimpleThermostatProxy::Release()
{
    LONG lRef = InterlockedDecrement( &m_cRef );

    if( 0 == lRef )
    {
        delete this;
    }
    return lRef;
}// CUPnPSimpleThermostatProxy::Release
