/*=====================================================================
File:      MachineActivation.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample shows how to Activate a machine. For backwards compatibility 
// with V1 clients, it takes an optional activation server URL as input.  
// It will use Service Discovery to find the activation server if a URL is 
// not provided.  See detailed comments at the beginning of wmain().
//

#include <stdio.h>
#include <Wtypes.h>
#include <strsafe.h>

#include "msdrm.h"
#include "msdrmdefs.h"
#include "msdrmerror.h"

//
// Time to wait for "downloads" to complete
//
static const DWORD DW_WAIT_TIME_SECONDS = 60 * 1000;

//
// struct to hold the callback information
//
typedef struct Drm_Context
{
    HANDLE  hEvent;
    HRESULT hr;
} DRM_CONTEXT, *PDRM_CONTEXT;

//
// Print the correct usage of this application
//
void 
PrintUsage()
{
    wprintf( L"Usage:\n" );
    wprintf( L"\n  MachineActivation [-A ActivationSvr]\n" );
    wprintf( L"    -A: Activation server URL. (optional)\n" );
    wprintf( L"        example: http://localhost/_wmcs/certification\n" );
}

//
// Parse the values passed in through the command line
//
HRESULT 
ParseCommandLine( 
                 int argc, 
                 __in_ecount( argc )WCHAR **argv, 
                 __deref_out PWCHAR *pwszActivationSvr 
                 )
{
    HRESULT hr = S_OK;
    size_t uiUrlLength = 0;

    //
    // Check input parameters.
    //
    if ( ( 3 != argc ) && ( 1 != argc ) )
    {
        return E_INVALIDARG;
    }

    for( int i = 1; SUCCEEDED( hr ) && i < argc - 1; i ++ )
    {
        if ( ( '-' != argv[ i ][ 0 ] ) && ( '/' != argv[ i ][ 0 ] ) )
        {
            hr = E_INVALIDARG;
            break;
        }
        if ( ( '-' == argv[ i + 1 ][ 0 ] ) || ( '/' == argv[ i + 1 ][ 0 ] ) )
        {
            hr = E_INVALIDARG;
            break;
        }

        //
        // Activation server URL
        //
        if ( 'A' == toupper( argv[ i ][ 1 ] ) )
        {
            if ( ( _wcsnicmp( argv[ i + 1 ], L"http://", 7 ) != 0 ) && 
                ( _wcsnicmp( argv[ i + 1 ], L"https://", 8 ) != 0 ) )
            {
                hr = E_INVALIDARG;
                break;
            }
            //
            // Retrieve the length of the activation server URL
            //
            hr = StringCchLengthW( argv[ i + 1 ], STRSAFE_MAX_CCH, &uiUrlLength );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the URL
            //
            *pwszActivationSvr = new WCHAR[ uiUrlLength + 1 ];
            if ( NULL == *pwszActivationSvr ) 
            {
                wprintf( L"Failed to allocate memory for pwszActivationSvr.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the URL into the pwszActivationSvr buffer
            //
            hr = StringCchCopyW( (wchar_t*)*pwszActivationSvr, uiUrlLength + 1 , argv[ i + 1 ] );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                break;
            }
        }
        else
        {
            hr = E_INVALIDARG;
            break;
        }
    }
    return hr;
}

//
// Callback function for asynchronous ADRMS client functions
//
HRESULT __stdcall 
StatusCallback( 
               DRM_STATUS_MSG msg, 
               HRESULT hr, 
               void *pvParam, 
               void *pvContext 
               )
{
    PDRM_CONTEXT pContext = ( PDRM_CONTEXT )pvContext;

    if ( pContext )
    {
        pContext->hr = hr;
    }
    else
    {
        return E_FAIL;
    }

    pvParam = NULL;

    if ( FAILED( pContext->hr ) && pContext->hEvent )
    {
        //
        // Signal the event
        //
        SetEvent( ( HANDLE )pContext->hEvent );
        return S_OK;
    }

    //
    // Print the callback status message
    //
    if ( DRM_MSG_ACTIVATE_MACHINE == msg )
    {
        wprintf( L"\nCallback status msg = DRM_MSG_ACTIVATE_MACHINE; " );
    }
    else
    {
        wprintf( L"\nDefault callback status msg = 0x%x ", msg );
    }

    //
    // Print the callback error code
    //
    switch( pContext->hr )
    {
    case S_DRM_ALREADY_ACTIVATED:
        wprintf( L"Callback hr = S_DRM_ALREADY_ACTIVATED\n" );
        break;
    case S_DRM_CONNECTING:
        wprintf( L"Callback hr = S_DRM_CONNECTING\n" );
        break;
    case S_DRM_CONNECTED:
        wprintf( L"Callback hr = S_DRM_CONNECTED\n" );
        break;
    case S_DRM_INPROGRESS:
        wprintf( L"Callback hr = S_DRM_INPROGRESS\n" );
        break;
    case S_DRM_COMPLETED:
        wprintf( L"Callback hr = S_DRM_COMPLETED\n" );
        pContext->hr = S_OK;
        if ( pContext->hEvent )
        {
            SetEvent( ( HANDLE )pContext->hEvent );
        }
        break;
    case E_DRM_ACTIVATIONFAILED:
        wprintf( L"Callback hr = E_DRM_ACTIVATIONFAILED\n" );
        break;
    case E_DRM_HID_CORRUPTED:
        wprintf( L"Callback hr = E_DRM_HID_CORRUPTED\n" );
        break;
    case E_DRM_INSTALLATION_FAILED:
        wprintf( L"Callback hr = E_DRM_INSTALLATION_FAILED\n" );
        break;
    case E_DRM_ALREADY_IN_PROGRESS:
        wprintf( L"Callback hr = E_DRM_ALREADY_IN_PROGRESS\n" );
        break;
    case E_DRM_NO_CONNECT:
        wprintf( L"Callback hr = E_DRM_NO_CONNECT\n" );
        break;
    case E_DRM_ABORTED:
        wprintf( L"Callback hr = E_DRM_ABORTED\n" );
        break;
    case E_DRM_SERVER_ERROR:
        wprintf( L"Callback hr = E_DRM_SERVER_ERROR\n" );
        break;
    case E_DRM_INVALID_SERVER_RESPONSE:
        wprintf( L"Callback hr = E_DRM_INVALID_SERVER_RESPONSE\n" );
        break;
    case E_DRM_SERVER_NOT_FOUND:
        wprintf( L"Callback hr = E_DRM_SERVER_NOT_FOUND\n" );
        break;
    default:
        wprintf( L"Default callback hr = 0x%x\n", hr );
        pContext->hr = hr;
        if ( pContext->hEvent )
        {
            SetEvent( ( HANDLE )pContext->hEvent );
        }
        break;
    }
    return S_OK;
}

//
// This sample will perform the following actions:
//    1.  Create an event for the callback function
//    2.  Create a client session
//    3.  Determine if the machine needs to be activated
//    4.  Populate the wszURL member of the DRM_ACTSERV_INFO struct
//        with the activation server URL.  This value can be from:
//        (a) a command line argument
//        (b) service discovery
//    5.  Activate the machine using DRMActivate
//    6.  Wait for the callback to return
//    7.  Perform any necessary memory clean up and deallocation
//
int __cdecl 
wmain( 
       int argc, 
       __in_ecount( argc )WCHAR **argv 
       )
{
    HRESULT           hr               = E_FAIL;
    DRM_ACTSERV_INFO *pdasi            = NULL;
    DRMHSESSION       hClient          = NULL;
    PWCHAR            wszActivationSvr = NULL;
    DWORD             dwWaitResult;
    UINT              uiStrLen         = 0;
    DRM_CONTEXT       context;

    context.hEvent = NULL;

    if ( FAILED( ParseCommandLine( argc, argv, &wszActivationSvr ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample Activate...\n" );

    //
    // 1. Create an event for StatusCallback() to signal it has completed.
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call" );
        goto e_Exit;
    }

    //
    // 2. Create a client session
    //
    hr = DRMCreateClientSession( &StatusCallback, 
                                 0, 
                                 DRM_DEFAULTGROUPIDTYPE_WINDOWSAUTH, 
                                 NULL, 
                                 &hClient 
                                 );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateClientSession failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 3. Call DRMIsActivated to determine if the machine is already activated
    //
    hr = DRMIsActivated( hClient, 
                         DRM_ACTIVATE_MACHINE, 
                         NULL 
                         );
    if ( SUCCEEDED( hr ) )
    {
        wprintf( L"\nThe machine is already activated.\n" );
        goto e_Exit;
    }
    //
    // Fail if the failure code was anything but E_DRM_NEEDS_MACHINE_ACTIVATION
    //
    if ( E_DRM_NEEDS_MACHINE_ACTIVATION != hr )
    {
        wprintf( L"\nDRMIsActivated returned an unexpected failure: 0x%x.  "\
                 L"E_DRM_NEEDS_MACHINE_ACTIVATION was expected.\n", hr );
        goto e_Exit;
    }

    //
    // Allocate memory for the DRM_ACTSERV_INFO structure for activation
    //
    pdasi = new DRM_ACTSERV_INFO;
    if ( NULL == pdasi )
    {
        wprintf( L"\nMemory allocation failed for DRM_ACTSERV_INFO.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    pdasi->wszURL = NULL;

    //
    // 4. Copy the activation server URL into the wszURL member
    //    of the DRM_ACTSERV_INFO struct 
    //
    if ( NULL != wszActivationSvr )
    {
        //
        // 4(a). Use the URL provided through the command line argument
        //
        pdasi->wszURL = wszActivationSvr;
        wszActivationSvr = NULL;

        wprintf( L"\nMachine Activation server URL:\n%s\n", pdasi->wszURL );
    }
    else
    {
        //
        // 4(b). Try to find the service location where activation 
        //       is available.
        //       The first call is to get -
        //       (a) whether there is a service location and 
        //       (b) if so, the size needed for a buffer to 
        //           hold the URL for the service.
        //
        hr = DRMGetServiceLocation( hClient,
                                    DRM_SERVICE_TYPE_ACTIVATION,
                                    DRM_SERVICE_LOCATION_ENTERPRISE,
                                    NULL,
                                    &uiStrLen,
                                    NULL 
                                    );

        if ( SUCCEEDED( hr ) )
        {
            //
            // There is a service location; reserve space
            // for the URL.
            //
            pdasi->wszURL = new WCHAR[ uiStrLen ];
            if ( NULL == pdasi->wszURL )
            {
                wprintf( L"\nMemory allocation failed for the activation "\
                         L"URL string.\n" );
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            //
            // Call a second time to get the actual service location
            // copied into the URL.
            //
            hr = DRMGetServiceLocation( hClient,
                                        DRM_SERVICE_TYPE_ACTIVATION,
                                        DRM_SERVICE_LOCATION_ENTERPRISE,
                                        NULL,
                                        &uiStrLen,
                                        pdasi->wszURL
                                        );

            if ( FAILED( hr ) )
            {
                wprintf( L"\nDRMGetServiceLocation (ENTERPRISE) failed. "\
                         L"hr = 0x%x\n", hr );
                goto e_Exit;
            }
            wprintf( L"\nDRMGetServiceLocation (ENTERPRISE) succeeded.\n\n"\
                     L"Machine Activation server URL:\n%s\n", pdasi->wszURL );
        }
        else
        {
            wprintf( L"\nDRMGetServiceLocation failed. hr = 0x%x. "\
                     L"\nPassing NULL server info to DRMActivate.\n", hr );
        }
    }

    //
    // 5. Activate the machine
    //
    hr = DRMActivate( hClient,
                      DRM_ACTIVATE_MACHINE | DRM_ACTIVATE_SILENT,
                      0,
                      pdasi,
                      ( VOID* )&context,
                      NULL 
                      );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMActivate (DRM_ACTIVATE_MACHINE) "\
                 L"failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 6. Wait for the callback to return
    //
    dwWaitResult = WaitForSingleObject( context.hEvent, DW_WAIT_TIME_SECONDS );
    if ( WAIT_TIMEOUT == dwWaitResult )
    {
        wprintf( L"\nWaitForSingleObject timed out." );
        goto e_Exit;
    }
    if ( FAILED( context.hr ) )
    {
        //
        // In case a failure was reported via the callback function
        // note it also.
        //
        wprintf( L"\nThe callback function returned a failure "\
                 L"code. hr = 0x%x\n", context.hr );
        hr = context.hr;
        goto e_Exit;
    }

    wprintf( L"\nThe machine is now activated.\n" );

e_Exit:
    //
    // 7. Clean up and free memory
    //
    if ( NULL != pdasi )
    {
        if ( NULL != pdasi->wszURL )
        {
            delete [] pdasi->wszURL;
        }
        delete pdasi;
    }
    if ( NULL != wszActivationSvr )
    {
        delete [] wszActivationSvr;
    }
    if ( NULL != hClient )
    {
        DRMCloseSession( hClient );
    }
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    if ( FAILED( hr ) )
    {
        wprintf( L"\nMachine Activation failed. hr = 0x%x\n", hr );
    }
    return hr;
}