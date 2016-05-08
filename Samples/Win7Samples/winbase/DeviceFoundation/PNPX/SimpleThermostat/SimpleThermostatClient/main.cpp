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
#include <functiondiscovery.h>

// Sample Headers
#include "SimpleThermostat.h"
#include "SimpleThermostatProxy.h"

//
// Function Declarations
//
VOID DeviceControl( 
    __in IFunctionInstance* pFunInst
    );

LONG GetDesiredTempFromUser();

HRESULT GetDeviceFiFromUser(
    __deref_out IFunctionInstance** ppFunInst
    );

HRESULT PrintDeviceFiInfo(
    __in IFunctionInstance* pFunInst
    );

//
// Globals
//
const ULONG GUID_CCH_LEN = 40;

//------------------------------------------------------------------------------
// wmain
//------------------------------------------------------------------------------
int __cdecl wmain( int argc, __in_ecount(argc) wchar_t* argv[] )
{
    HRESULT             hr          = E_FAIL;
    IFunctionInstance*  pFunInst    = NULL;

    wprintf( L"\n" );
    wprintf( L"*** Simple Thermostat Client ***\n\n" );
    wprintf( L"Searching for installed ISimpleThermostat devices...\n\n" );

    wprintf( L"CoInitializeEx..." );
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    wprintf( L"0x%x\n", hr );

    //
    // Search for ISimpleThermostat devices in PnP and ask the user
    // which one to use.
    //
    if( S_OK == hr )
    {
        hr = GetDeviceFiFromUser( &pFunInst );
    }

    //
    // Allow the user to interact with the selected device
    //
    if( S_OK == hr )
    {
        DeviceControl( pFunInst );
    }

    wprintf( L"Exiting...\n" );

    //
    // Cleanup
    //
    if( NULL != pFunInst )
    {
        pFunInst->Release();
        pFunInst = NULL;
    }

    return hr;

}// wmain


//------------------------------------------------------------------------------
// DeviceControl
//      Presents the user with information about the device represented by the
//      FI. Also allows minimum interaction with the device.
//------------------------------------------------------------------------------
VOID DeviceControl( 
    IFunctionInstance* pFunInst
    )
{
    HRESULT             hr                  = S_OK;
    ULONG               i                   = 0;
    LONG                lCurrentTemp        = 0;
    LONG                lDesiredTemp        = 0;
    PROPVARIANT         pvID                = {0};
    PROPVARIANT         pvName              = {0};
    IPropertyStore*     pPropStore          = NULL;
    ISimpleThermostat*  pSimpleThermostat   = NULL;

    //
    // Ask for the simple thermostat object from the function instance
    //
    wprintf( L"Query service on the FI for ISimpleThermostat...\n" );
    hr = pFunInst->QueryService(
        __uuidof(SimpleThermostatProxy),
        __uuidof(ISimpleThermostat),
        reinterpret_cast<void**>(&pSimpleThermostat)
        );
    wprintf( L"Query service for ISimpleThermostat returned...0x%x\n", hr );

    //
    // Grab the name and ID of the device
    //
    if( S_OK == hr )
    {
        wprintf( L"OpenPropertyStore on the FI..." );
        hr = pFunInst->OpenPropertyStore( STGM_READ, &pPropStore );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )   
    {
        wprintf( L"GetValue of PKEY_PNPX_FriendlyName..." );
        hr = pPropStore->GetValue( PKEY_PNPX_FriendlyName, &pvName );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        wprintf( L"GetValue of PKEY_PNPX_GlobalIdentity..." );
        hr = pPropStore->GetValue( PKEY_PNPX_GlobalIdentity, &pvID );
        wprintf( L"0x%x\n", hr );
    }

    //
    // Respond to commands from the user until they choose to exit
    //
    while( S_OK == hr &&
           i != 3 )
    {
        wprintf( L"GetCurrentTemp from thermostat (remote call)..." );
        hr = pSimpleThermostat->GetCurrentTemp( &lCurrentTemp );
        wprintf( L"0x%x\n", hr );

        wprintf( L"GetDesiredTemp from thermostat (remote call)..." );
        hr = pSimpleThermostat->GetDesiredTemp( &lDesiredTemp );
        wprintf( L"0x%x\n\n", hr );

        wprintf( L" __________________________\n" );
        wprintf( L"| Simple Thermostat Device |\n" );
        wprintf( L"|__________________________|_________________________\n" );
        wprintf( L"|                                                    |\n" );
        wprintf( L"|  Name: %-43.43s |\n", pvName.pwszVal );
        wprintf( L"|  ID: %-45.45s |\n", pvID.pwszVal );
        wprintf( L"|  Current Temp: %-35d |\n", lCurrentTemp );
        wprintf( L"|  Desired Temp: %-35d |\n", lDesiredTemp );
        wprintf( L"|                                                    |\n" );
        wprintf( L"| MENU                                               |\n" );
        wprintf( L"|  1) Refresh Device Info                            |\n" );
        wprintf( L"|  2) Set Desired Temp                               |\n" );
        wprintf( L"|  3) Exit                                           |\n" );
        wprintf( L"|____________________________________________________|\n" );
        wprintf( L"> " );
        i = _getche() - '0';
        wprintf( L"\n\n" );

        switch( i )
        {
        case 1:
            wprintf( L"Refreshing device info...\n" );
            //
            // Nothing needs to be done here since the device info is refreshed
            // with every loop.
            //
            break;
        case 2:
            lDesiredTemp = GetDesiredTempFromUser();
            wprintf( L"SetDesiredTemp on thermostat (remote call)..." );
            hr = pSimpleThermostat->SetDesiredTemp( lDesiredTemp );
            wprintf( L"0x%x\n", hr );
            break;
        case 3:
            break;
        default:
            wprintf( L"Invalid selection...\n" );
            break;
        }
    }

    //
    // Cleanup
    //
    PropVariantClear( &pvID );
    PropVariantClear( &pvName );

    if( NULL != pPropStore )
    {
        pPropStore->Release();
        pPropStore = NULL;
    }

    if( NULL != pSimpleThermostat )
    {
        pSimpleThermostat->Release();
        pSimpleThermostat = NULL;
    }

    return;
}// DeviceControl



//------------------------------------------------------------------------------
// GetDesiredTempFromUser
//      Prompts the user to input a desired temperature and returns the value to
//      the caller.
//------------------------------------------------------------------------------
LONG GetDesiredTempFromUser()
{
    HRESULT hr              = S_OK;
    LONG    lDesiredTemp    = 0;
    WCHAR   szTemp[5]       = {0};

    wprintf( L"Enter Desired Temp (4 digit max)> " );

    hr = StringCchGetsW( szTemp, 5 );

    wprintf( L"\n" );

    if( S_OK == hr )
    {
        lDesiredTemp = _wtol( szTemp );
    }
    else
    {
        wprintf( L"Failed to get input string, hr = 0x%x", hr );
    }

    return lDesiredTemp;
}// GetDesiredTempFromUser


//------------------------------------------------------------------------------
// GetDeviceFiFromUser
//      Presents the user with a list of ISimpleThermostat devices and returns
//      the FI representing the device the user chooses.
//------------------------------------------------------------------------------
HRESULT GetDeviceFiFromUser(
    IFunctionInstance** ppFunInst
    )
{
    ULONG                               ulCount                     = 0;
    HRESULT                             hr                          = S_OK;
    ULONG                               i                           = 0;
    IFunctionDiscovery*                 pFunDisc                    = NULL;
    IFunctionInstance*                  pFunInst                    = NULL;
    IFunctionInstanceCollection*        pFunInsts                   = NULL;
    IFunctionInstanceCollectionQuery*   pFunInstsQuery              = NULL;
    WCHAR                               szInterface[GUID_CCH_LEN]   = {0};

    wprintf( L"CoCreate the Function Discovery object..." );
    hr = CoCreateInstance(
        __uuidof(FunctionDiscovery),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IFunctionDiscovery),
        reinterpret_cast<void**>(&pFunDisc)
        );
    wprintf( L"0x%x\n", hr );

    if ( S_OK == hr )
    {
        wprintf( L"Create an PnP instance collection query..." );
        hr = pFunDisc->CreateInstanceCollectionQuery(
            FCTN_CATEGORY_PNP,
            NULL,
            FALSE,
            NULL,
            NULL,
            &pFunInstsQuery
            );
        wprintf( L"0x%x\n", hr );
    }

    if ( S_OK == hr )
    {
        wprintf( L"Adding ISimpleThermostat as an interface constraint on the query..." );
        StringFromGUID2( __uuidof(ISimpleThermostat), szInterface, GUID_CCH_LEN );
        hr = pFunInstsQuery->AddQueryConstraint(
            PROVIDERPNP_QUERYCONSTRAINT_INTERFACECLASS,
            szInterface
            );
        wprintf( L"0x%x\n", hr );
    }

    if ( S_OK == hr )
    {
        wprintf( L"Execute the query..." );
        hr = pFunInstsQuery->Execute( &pFunInsts );
        wprintf( L"0x%x\n", hr );
    }

    if( S_OK == hr )
    {
        wprintf( L"Get the number of results..." );
        hr = pFunInsts->GetCount( &ulCount );
        wprintf( L"0x%x\n\n", hr );

        wprintf( L"RESULTS (Only listing first 5)\n\n" );
        wprintf( L"  #  Friendly Name              Device Identity\n" );
        wprintf( L"  -  -------------------------  ---------------------------------------------\n" );

        if( S_OK == hr &&
            ulCount < 1 )
        {
            wprintf( L"  No ISimpleThermostat devices found\n" );
            hr = E_FAIL;
        }

        if( S_OK == hr )
        {
            for( i = 0; i < ulCount; i++ )
            {
                hr = pFunInsts->Item( i, &pFunInst );
                wprintf( L"  %d) ", i+1 );
                if( S_OK == hr )
                {
                    hr = PrintDeviceFiInfo( pFunInst );
                }
                if( S_OK != hr )
                {
                    wprintf( L"Error retrieving device info" );
                }
                wprintf( L"\n" );
                hr = S_OK;
            }
        }
        wprintf( L"  ---------------------------------------------------------------------------\n" );
    }

    if( S_OK == hr &&
        ulCount > 0 )
    {
        wprintf( L"  Select a device> " );
        i = _getche() - '0';
        wprintf( L"\n\n" );
        if( 0 >= i ||
            ulCount < i )
        {
            wprintf( L"Invalid selection...\n" );
            hr = E_FAIL;
        }
    }

    if( S_OK == hr )
    {
        wprintf( L"Grabbing device %d...", i );
        hr = pFunInsts->Item( i-1, &pFunInst );
        wprintf( L"0x%x\n", hr );
        *ppFunInst = pFunInst;
        pFunInst->AddRef();
    }

    //
    // Cleanup
    //
    if( NULL != pFunInst )
    {
        pFunInst->Release();
        pFunInst = NULL;
    }

    if( NULL != pFunInsts )
    {
        pFunInsts->Release();
        pFunInsts = NULL;
    }

    if( NULL != pFunInstsQuery )
    {
        pFunInstsQuery->Release();
        pFunInstsQuery = NULL;
    }

    if( NULL != pFunDisc )
    {
        pFunDisc->Release();
        pFunDisc = NULL;
    }

    return hr;
}// GetDeviceFiFromUser


//------------------------------------------------------------------------------
// PrintDeviceFiInfo
//      Prints basic information about a device to the console.
//------------------------------------------------------------------------------
HRESULT PrintDeviceFiInfo(
    IFunctionInstance* pFunInst
    )
{
    HRESULT         hr          = S_OK;
    PROPVARIANT     pvName      = {0};
    PROPVARIANT     pvID        = {0};
    IPropertyStore* pPropStore  = NULL;

    if( NULL == pFunInst )
    {
        return E_INVALIDARG;
    }

    hr = pFunInst->OpenPropertyStore( STGM_READ, &pPropStore );

    if( S_OK == hr )   
    {
        hr = pPropStore->GetValue( PKEY_PNPX_FriendlyName, &pvName );
    }

    if( S_OK == hr )
    {
        hr = pPropStore->GetValue( PKEY_PNPX_GlobalIdentity, &pvID );
    }

    if( S_OK == hr &&
        VT_LPWSTR == pvName.vt &&
        VT_LPWSTR == pvID.vt )
    {
        wprintf( L"%-25.25s  %-45.45s", pvName.pwszVal, pvID.pwszVal );
    }

    //
    // Cleanup
    //
    PropVariantClear( &pvName );
    PropVariantClear( &pvID );

    if( NULL != pPropStore )
    {
        pPropStore->Release();
        pPropStore = NULL;
    }

    return hr;
}// PrintDeviceFiInfo