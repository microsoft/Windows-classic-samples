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

// Sample Headers
#include "CSimpleThermostatService.h"
#include "SimpleThermostat_WSDTypes.h"

//
// Globals
//
ULONG g_ulInstanceID = 0;

//
// Device metadata.
// To edit your model metadata, edit the codegen.config file.
//
WSD_LOCALIZED_STRING thisDeviceName = {NULL, L"WSD Simple Thermostat"};
WSD_LOCALIZED_STRING_LIST thisDeviceNameList = {NULL, &thisDeviceName};

const WSD_THIS_DEVICE_METADATA thisDeviceMetadata = {
    &thisDeviceNameList,    // FriendlyName
    L"0.145",               // FirmwareVersion
    L"5123456789",          // SerialNumber
};

//
// Function declarations
//
VOID ReceiveKeyboardCommands( __in IWSDDeviceHost* pDeviceHost );

//------------------------------------------------------------------------------
// wmain
//------------------------------------------------------------------------------
int __cdecl wmain( int argc, __in_ecount(argc) wchar_t* argv[] )
{
    HRESULT                     hr                          = S_OK;
    IWSDXMLContext*             pContext                    = NULL;
    IWSDDeviceHost*             pDeviceHost                 = NULL;
    CSimpleThermostatService*   pCSimpleThermostatService   = NULL;
    WCHAR                       szDeviceAddress[MAX_PATH]   = {0};
    UUID                        uuid                        = {0};

    //
    // A uuid is required to create a WSD device host. This id not only
    // acts as the unique id of the device, but it's also the device's
    // virtual address.
    //
    wprintf( L"Create device host id..." );
    hr = UuidCreate( &uuid );
    if( S_OK == hr )
    {
        hr = StringCbPrintfW(
            szDeviceAddress, sizeof(szDeviceAddress),
            L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid.Data1, uuid.Data2, uuid.Data3,
            uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
            uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    }
    wprintf( L"0x%x\n", hr );

    if( S_OK == hr )
    {
        wprintf( L"Device host id: %s\n", szDeviceAddress );
    }

    //
    // Create the service object that will be plugged into the device host.
    //
    if( S_OK == hr )
    {
        wprintf( L"Create the CSimpleThermostatService object..." );
        pCSimpleThermostatService = new CSimpleThermostatService();
        if( NULL == pCSimpleThermostatService )
        {
            hr = E_OUTOFMEMORY;
        }
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        wprintf( L"Create WSD device host..." );
        hr = CreateSimpleThermostat_WSDHost(
            szDeviceAddress,
            &thisDeviceMetadata,
            pCSimpleThermostatService,
            &pDeviceHost,
            &pContext
            );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Start the device host. This sends a HELLO message on the network.
    //
    if( S_OK == hr )
    {
        wprintf( L"Starting device host..." );
        hr = pDeviceHost->Start( g_ulInstanceID, NULL, NULL );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Receive keyboard commands from the user.
    //
    if( S_OK == hr )
    {
        ReceiveKeyboardCommands( pDeviceHost );
    }

    //
    // Cleanup
    //
    if( NULL != pDeviceHost )
    {
        pDeviceHost->Stop();
        pDeviceHost->Terminate();
        pDeviceHost->Release();
        pDeviceHost = NULL;
    }

    if( NULL != pCSimpleThermostatService )
    {
        pCSimpleThermostatService->Release();
        pCSimpleThermostatService = NULL;
    }

    if( NULL != pContext )
    {
        pContext->Release();
        pContext = NULL;
    }

    return hr;
}// wmain


//------------------------------------------------------------------------------
// ReceiveKeyboardCommands
//      Function receives commmands through the keyboard to control the device.
//      This function runs until its signaled to exit by pressing the 'q' key.
//------------------------------------------------------------------------------
VOID ReceiveKeyboardCommands( IWSDDeviceHost* pDeviceHost )
{
    BOOL    bExit   = FALSE;
    HRESULT hr      = S_OK;
    int     i       = 0;

    while( FALSE == bExit )
    {
        wprintf( L"Press s = start, p = stop, q = quit\n" );
        i = _getch();
        switch( i )
        {
        case 's':
        case 'S':
            wprintf( L"Starting device host..." );
            hr = pDeviceHost->Start( g_ulInstanceID, NULL, NULL  );
            wprintf( L"0x%x\n", hr );
            break;
        case 'p':
        case 'P':
            wprintf( L"Stopping device host..." );
            hr = pDeviceHost->Stop();
            wprintf( L"0x%x\n", hr );
            //
            // According to spec., the instance id needs to be incremented
            // every time the device is restarted. The value is incremented
            // here in case the device is started again.
            //
            g_ulInstanceID++;
            break;
        case 'q':
        case 'Q':
            wprintf( L"Exiting..." );
            bExit = TRUE;
            break;
        }
    }

    return;
}// ReceiveKeyboardCommands

