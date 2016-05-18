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
#include <conio.h>
#include <strsafe.h>
#include <upnphost.h>
#include <msxml6.h>

// Sample Headers
#include "SimpleThermostatDevice.h"

//
// Globals
//
const WCHAR DEVICE_UDN[] = L"uuid:2593cc72-9adb-4e17-8e30-1f00f14d7096";
const WCHAR DEVICE_XML_DOC[] = L"SimpleThermostatDevice.xml";

//
// Function declarations
//
HRESULT CreateUPnPDevice(
    __in PCWSTR pszXMLDoc,
    __deref_out BSTR* pbstrDeviceId,
    __deref_out BSTR* pbstrXMLDoc,
    __deref_out BSTR* pbstrResource,
    __deref_out IUnknown** ppDevice,
    __deref_out IUPnPRegistrar** ppUPnPRegistrar
    );

HRESULT ReceiveKeyboardCommands(
    __in BSTR bstrDeviceId,
    __in BSTR bstrXMLDoc,
    __in BSTR bstrResource,
    __in IUnknown* pDevice,
    __in IUPnPRegistrar* pUPnPRegistrar
    );

//------------------------------------------------------------------------------
// wmain
//------------------------------------------------------------------------------
int __cdecl wmain( int argc, __in_ecount(argc) wchar_t* argv[] )
{
    BSTR            bstrDeviceId      = NULL;
    BSTR            bstrResource      = NULL;
    BSTR            bstrTempUDN       = NULL;
    BSTR            bstrUniqueId      = NULL;
    BSTR            bstrXMLDoc        = NULL;
    HRESULT         hr                = E_FAIL;
    IUnknown*       pDevice           = NULL;
    IUPnPRegistrar* pUPnPRegistrar    = NULL;

    wprintf( L"CoInitialize..." );
    hr = CoInitialize( NULL );
    wprintf( L"0x%x\n", hr );

    if( S_OK == hr )
    {
        wprintf( L"CoInitialize Security..." );
        hr = CoInitializeSecurity(
                NULL,
                -1,
                NULL,
                NULL,
                RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL,
                EOAC_SECURE_REFS|EOAC_DISABLE_AAA,
                NULL
                );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Get the device registered with UPnP and get the registrar control
    // object to it.
    //
    if( S_OK == hr )
    {
        hr = CreateUPnPDevice(
            DEVICE_XML_DOC,
            &bstrDeviceId,
            &bstrXMLDoc,
            &bstrResource,
            &pDevice,
            &pUPnPRegistrar
            );
    }

    if( S_OK == hr )
    {
        bstrTempUDN = SysAllocString( DEVICE_UDN );
        if( NULL == bstrTempUDN )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    //
    // The UDN supplied by this code (and the device's XML document)
    // is not the real UDN the UPnPHost exposes on the network. To
    // get the real UDN generated, GetUniqueDeviceName is called.
    //
    if( S_OK == hr )
    {
        wprintf( L"Get the device's unique id..." );
        hr = pUPnPRegistrar->GetUniqueDeviceName(
            bstrDeviceId,
            bstrTempUDN,
            &bstrUniqueId
            );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        wprintf( L"Device host id: %s\n", bstrUniqueId );
    }

    //
    // Receive keyboard commands from the user.
    //
    if( S_OK == hr )
    {
        ReceiveKeyboardCommands(
            bstrDeviceId,
            bstrXMLDoc,
            bstrResource,
            pDevice,
            pUPnPRegistrar
            );
    }

    //
    // Cleanup
    //
    if( NULL != pUPnPRegistrar )
    {
        pUPnPRegistrar->UnregisterDevice( bstrDeviceId, TRUE );
        pUPnPRegistrar->Release();
        pUPnPRegistrar = NULL;
    }

    if( NULL != bstrDeviceId )
    {
        SysFreeString( bstrDeviceId );
        bstrDeviceId = NULL;
    }

    if( NULL != bstrResource )
    {
        SysFreeString( bstrResource );
        bstrResource = NULL;
    }

    if( NULL != bstrTempUDN )
    {
        SysFreeString( bstrTempUDN );
        bstrTempUDN = NULL;
    }

    if( NULL != bstrUniqueId )
    {
        SysFreeString( bstrUniqueId );
        bstrUniqueId = NULL;
    }

    if( NULL != bstrXMLDoc )
    {
        SysFreeString( bstrXMLDoc );
        bstrXMLDoc = NULL;
    }

    if( NULL != pDevice )
    {
        pDevice->Release();
        pDevice = NULL;
    }

    CoUninitialize();

    return hr;

}// wmain


//------------------------------------------------------------------------------
// CreateUPnPDevice
//      This function creates the UPnP simple thermostat device object (the one
//      created in the UPnPSimpleThermostatDeviceDLL project)
//      registers it device with UPnP, and then returns the UPnPRegistrar
//      control object to the caller.
//------------------------------------------------------------------------------
HRESULT CreateUPnPDevice(
    PCWSTR pszXMLDoc,
    BSTR* pbstrDeviceId,
    BSTR* pbstrXMLDoc,
    BSTR* pbstrResource,
    IUnknown** ppDevice,
    IUPnPRegistrar** ppUPnPRegistrar
    )
{
    BSTR                bstrXMLDoc              = NULL;
    BSTR                bstrResource            = NULL;
    HRESULT             hr                      = S_OK;
    IXMLDOMDocument*    pXMLDoc                 = NULL;
    IUnknown*           pDevice                 = NULL;
    IUPnPRegistrar*     pUPnPRegistrar          = NULL;
    WCHAR               szResource[MAX_PATH]    = {0};
    ULONG               ulLifetime              = 1500;
    VARIANT             var                     = {0};
    VARIANT_BOOL        vbStatus;

    //
    // Create the DOM Document object. This is just used so the device's entire
    // description document can be easily grabbed as a BSTR. The BSTR will be
    // used when the device is registered with UPnP.
    //
    wprintf( L"Create the DOM Object..." );
    hr = CoCreateInstance(
        CLSID_DOMDocument30,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IXMLDOMDocument,
        (void**)&pXMLDoc
        );
    wprintf( L"0x%x\n", hr );

    if( S_OK == hr )
    {
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString( pszXMLDoc );
        if( NULL == var.bstrVal )
        {
            var.vt = VT_EMPTY;
            hr = E_OUTOFMEMORY;
        }
    }

    //
    // Load the device's description document with the DOM object
    //
    if( S_OK == hr )
    {
        wprintf( L"Load the upnp device description document..." );
        hr = pXMLDoc->load( var, &vbStatus );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Get the BSTR equivalent of the description document
    //
    if( S_OK == hr )
    {
        wprintf( L"Grab the bstr string of the device description document..." );
        hr = pXMLDoc->get_xml( &bstrXMLDoc );
        wprintf( L"0x%x\n", hr );
    }

    //
    // CoCreate the simple thermostat device object from the
    // UPnPSimpleThermostatDeviceDLL project.
    //
    if( S_OK == hr )
    {
        wprintf( L"CoCreate the simple thermostat device object..." );
        hr = CoCreateInstance(
            __uuidof(SimpleThermostatDevice),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IUnknown),
            (LPVOID*)&pDevice
            );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Create the registrar object that will be used to register the above
    // device object with UPnP.
    //
    if( S_OK == hr )
    {
        wprintf( L"Create the UPnP registrar object..." );
        hr = CoCreateInstance(
            __uuidof(UPnPRegistrar),
            NULL,
            CLSCTX_LOCAL_SERVER,
            __uuidof(IUPnPRegistrar),
            (LPVOID *)&pUPnPRegistrar
            );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        if( 0 == GetCurrentDirectoryW( MAX_PATH, szResource ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        else if( NULL == ( bstrResource = SysAllocString( szResource ) ) )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    //
    // Register the device object with UPnP.
    //
    if( S_OK == hr )
    {
        wprintf( L"RegisterDevice..." );
        hr = pUPnPRegistrar->RegisterRunningDevice( 
            bstrXMLDoc, 
            pDevice,
            bstrResource,
            bstrResource,
            ulLifetime,
            pbstrDeviceId
            );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Cleanup
    //
    if( S_OK == hr )
    {
        *pbstrXMLDoc = bstrXMLDoc;
        *pbstrResource = bstrResource;
        *ppDevice = pDevice;
        *ppUPnPRegistrar = pUPnPRegistrar;
    }
    else
    {
        if( NULL != bstrXMLDoc )
        {
            SysFreeString( bstrXMLDoc );
            bstrXMLDoc = NULL;
        }

        if( NULL != bstrResource )
        {
            SysFreeString( bstrResource );
            bstrResource = NULL;
        }

        if( NULL != pDevice )
        {
            pDevice->Release();
            pDevice = NULL;
        }

        if( NULL != pUPnPRegistrar )
        {
            pUPnPRegistrar->Release();
            pUPnPRegistrar = NULL;
        }
    }

    VariantClear( &var );

    if( NULL != pXMLDoc )
    {
        pXMLDoc->Release();
        pXMLDoc = NULL;
    }

    return hr;
}// CreateUPnPDevice


//------------------------------------------------------------------------------
// ReceiveKeyboardCommands
//      Function receives commmands through the keyboard to control the device.
//      This function runs until its signaled to exit by pressing the 'q' key.
//------------------------------------------------------------------------------
HRESULT ReceiveKeyboardCommands(
    BSTR bstrDeviceId,
    BSTR bstrXMLDoc,
    BSTR bstrResource,
    IUnknown* pDevice,
    IUPnPRegistrar* pUPnPRegistrar
    )
{
    BOOL                bExit               = FALSE;
    HRESULT             hr                  = S_OK;
    IUPnPReregistrar*   pUPnPReregistrar    = NULL;
    ULONG               ulLifetime          = 1800;

    //
    // QI for the IUPnPReregistrar interface so it can be used to restart
    // the device after it's already running.
    //
    hr = pUPnPRegistrar->QueryInterface(
        __uuidof(IUPnPReregistrar),
        (void**)&pUPnPReregistrar
        );

    if( S_OK == hr )
    {
        while( FALSE == bExit )
        {
            wprintf( L"Press s = start, p = stop, q = quit\n" );
            int ch = _getch();
            switch( ch )
            {
            case 's':
            case 'S':
                wprintf( L"Starting device host..." );
                hr = pUPnPReregistrar->ReregisterRunningDevice(
                    bstrDeviceId,
                    bstrXMLDoc,
                    pDevice,
                    bstrResource,
                    bstrResource,
                    ulLifetime
                    );
                wprintf( L"0x%x\n", hr );
                break;
            case 'p':
            case 'P':
                wprintf( L"Stopping device host..." );
                hr = pUPnPRegistrar->UnregisterDevice( bstrDeviceId, FALSE );
                wprintf( L"0x%x\n", hr );
                break;
            case 'q':
            case 'Q':
                wprintf( L"Exiting..." );
                bExit = TRUE;
                break;
            }
        }
    }

    if( NULL != pUPnPReregistrar )
    {
        pUPnPReregistrar->Release();
        pUPnPReregistrar = NULL;
    }

    return hr;
}// ReceiveKeyboardCommands
