/*=====================================================================
File:      acquireclientlicensor.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample shows how to acquire a client licensor certificate. It 
// takes an optional activation and certification server URL as input.
// It will use Service Discovery to find the server URLs if the URLs are
// not provided.  The sample requires that a UserID be provided as input.
// See the comments at the beginning of wmain() for a more detailed 
// description.
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
    wprintf( L"\n  AcquireClientLicensor -U UserID [-A ActivationSvr] "\
		     L"[-L LicensingSvr]\n" );
    wprintf( L"    -U: specifies the UserID.\n" );
    wprintf( L"        example: user@yourdomain.com\n" );
    wprintf( L"    -A: specifies the Activation Server. (optional)\n" );
    wprintf( L"        example: http://localhost/_wmcs/certification\n" );
	wprintf( L"    -L: specifies the Licensing Server. (optional)\n" );
	wprintf( L"        example: http://localhost/_wmcs/licensing\n" );
}

//
// Parse the values passed in through the command line
//
HRESULT 
ParseCommandLine( 
                 int argc, 
                 __in_ecount( argc )WCHAR **argv, 
                 __deref_out_opt PWCHAR *pwszUserID,
                 __deref_out PWCHAR *pwszActivationSvr,
                 __deref_out PWCHAR *pwszLicensingSvr
                 )
{
    HRESULT hr = S_OK;
    size_t uiUserIDLength = 0;
    size_t uiActSvrUrlLength = 0;
    size_t uiLicSvrUrlLength = 0;

    //
    // Initialize parameters
    //
    *pwszUserID = NULL;

    //
    // Check input parameters.
    //
    if ( ( 3 != argc ) && ( 5 != argc ) && ( 7 != argc ) )
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
        else if ( ( '-' == argv[ i + 1 ][ 0 ] ) || 
            ( '/' == argv[ i + 1 ][ 0 ] ) )
        {
            hr = E_INVALIDARG;
            break;
        }

        switch( toupper( argv[ i ][ 1 ] ) )
        {
            //
            // User ID
            //
        case 'U': 
            if ( wcsstr( argv[ i + 1 ], ( wchar_t* )L"@\0" ) == NULL )
            {
                //
                // An invalid User ID was provided
                //
                hr = E_INVALIDARG;
                break;
            }
            //
            // Retrieve the length of the user ID
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiUserIDLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the user ID
            //
            *pwszUserID = new WCHAR[ uiUserIDLength + 1 ];
            if ( NULL == *pwszUserID ) 
            {
                wprintf( L"Failed to allocate memory for pwszUserID.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the URL into the pwszUserID buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszUserID, 
                uiUserIDLength + 1 , 
                argv[ i + 1 ] 
                );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        case 'A':
            if ( ( _wcsnicmp( argv[ i + 1 ], L"http://", 7 ) != 0 ) && 
                ( _wcsnicmp( argv[ i + 1 ], L"https://", 8 ) != 0 ) )
            {
                wprintf( L"Invalid activation URL provided.\n" );
                hr = E_INVALIDARG;
                break;
            }
            //
            // Retrieve the length of the activation server URL
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiActSvrUrlLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the URL
            //
            *pwszActivationSvr = new WCHAR[ uiActSvrUrlLength + 1 ];
            if( NULL == *pwszActivationSvr ) 
            {
                wprintf( L"Failed to allocate memory "\
                    L"for pwszActivationSvr.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the URL into the pwszActivationSvr buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszActivationSvr, 
                uiActSvrUrlLength + 1 , 
                argv[ i + 1 ] 
                );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        case 'L':
            if ( ( _wcsnicmp( argv[ i + 1 ], L"http://", 7 ) != 0 ) && 
                ( _wcsnicmp( argv[ i + 1 ], L"https://", 8 ) != 0 ) )
            {
                wprintf( L"Invalid licensing URL provided.\n" );
                hr = E_INVALIDARG;
                break;
            }
            //
            // Retrieve the length of the licensing server URL
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiLicSvrUrlLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the URL
            //
            *pwszLicensingSvr = new WCHAR[ uiLicSvrUrlLength + 1 ];
            if( NULL == *pwszLicensingSvr ) 
            {
                wprintf( L"Failed to allocate memory "\
                    L"for pwszLicensingSvr.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the URL into the pwszLicensingSvr buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszLicensingSvr, 
                uiLicSvrUrlLength + 1 , 
                argv[ i + 1 ] 
                );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        default:
            hr = E_INVALIDARG;
            break;
        }
    }
    if ( NULL == *pwszUserID )
    {
        wprintf( L"A user ID value is required.\n" );
        hr = E_INVALIDARG;
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

    //
    // Set pvParam to NULL since we don't expect 
    // a return value from the callback
    //
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
    switch( msg )
    {
    case DRM_MSG_ACTIVATE_MACHINE:
        wprintf( L"\nCallback status msg = DRM_MSG_ACTIVATE_MACHINE " );
        break;
    case DRM_MSG_ACTIVATE_GROUPIDENTITY:
        wprintf( L"\nCallback status msg = DRM_MSG_ACTIVATE_GROUPIDENTITY " );
        break;
	case DRM_MSG_ACQUIRE_CLIENTLICENSOR:
		wprintf( L"\nCallback status msg = DRM_MSG_ACQUIRE_CLIENTLICENSOR " );
		break;
    default:
        wprintf( L"\nDefault callback status msg = 0x%x ", msg );
        break;
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
    case E_DRM_AUTHENTICATION_FAILED:
        wprintf( L"Callback hr = E_DRM_AUTHENTICATION_FAILED\n" );
        break;
    case E_DRM_EMAIL_NOT_VERIFIED:
        wprintf( L"Callback hr = E_DRM_EMAIL_NOT_VERIFIED\n" );
        break;
    case E_DRM_AD_ENTRY_NOT_FOUND:
        wprintf( L"Callback hr = E_DRM_AD_ENTRY_NOT_FOUND\n" );
        break;
    case E_DRM_NEEDS_MACHINE_ACTIVATION:
        wprintf( L"Callback hr = E_DRM_NEEDS_MACHINE_ACTIVATION\n" );
        break;
    case E_DRM_REQUEST_DENIED:
        wprintf( L"Callback hr = E_DRM_REQUEST_DENIED\n" );
        break;
    case E_DRM_INVALID_EMAIL:
        wprintf( L"Callback hr = E_DRM_INVALID_EMAIL\n" );
        break;
    case E_DRM_SERVICE_GONE:
        wprintf( L"Callback hr = E_DRM_SERVICE_GONE\n" );
        break;
    case E_DRM_SERVICE_MOVED:
        wprintf( L"Callback hr = E_DRM_SERVICE_MOVED\n" );
        break;
    case E_DRM_VALIDITYTIME_VIOLATION:
        wprintf( L"Callback hr = E_DRM_VALIDITYTIME_VIOLATION\n" );
        break;
    case S_DRM_REQUEST_PREPARED:
        wprintf( L"Callback hr = S_DRM_REQUEST_PREPARED\n" );
        break;
    case E_DRM_NO_LICENSE:
        wprintf( L"Callback hr = E_DRM_NO_LICENSE\n" );
        break;
    case E_DRM_SERVICE_NOT_FOUND:
        wprintf( L"Callback hr = E_DRM_SERVICE_NOT_FOUND\n" );
        break;
    default:
        wprintf( L"Default callback hr = 0x%x\n", hr );
        pContext->hr = S_OK;
        if ( pContext->hEvent )
        {
            SetEvent( ( HANDLE )pContext->hEvent );
        }
        break;
    }
    return S_OK;
}

//
// This function performs the following actions:
//    1.  Validate the parameters
//    2.  Create an event for the callback function
//    3.  Populate the wszURL member of the DRM_ACTSERV_INFO struct
//        with the activation server URL.  This value can be from:
//        (a) a command line argument
//        (b) service discovery
//    4.  Activate the machine
//    5. Wait for the callback to return
//    6. Clean up and free memory
HRESULT __stdcall 
DoMachineActivation(
                    DRMHSESSION hClient,
                    __in PWCHAR wszActivationSvr
                    )
{
    HRESULT           hr               = E_FAIL;
    DRM_ACTSERV_INFO *pdasi            = NULL;
    UINT              uiStrLen         = 0;
    size_t            cchActivationSvr = 0;
    DWORD             dwWaitResult;
    DRM_CONTEXT       context;

    context.hEvent = NULL;

    //
    // 1. Validate the parameters
    //
    if ( NULL == hClient )
    {
        wprintf( L"\nThe client session was NULL." );
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // 2. Create an event for the callback function
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call" );
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
    // 3. Copy the activation server URL into the wszURL member
    //    of the DRM_ACTSERV_INFO struct 
    //
    if ( NULL != wszActivationSvr )
    {
        //
        // 3(a). Use the URL provided through the command line argument
        //
        hr = StringCchLengthW( wszActivationSvr, 
            STRSAFE_MAX_CCH, 
            &cchActivationSvr 
            );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nStringCchLengthW failed. hr = 0x%x.\n", hr );
            goto e_Exit;
        }

        //
        // Allocate memory for the URL member of the DRM_ACTSERV_INFO structure
        //
        pdasi->wszURL = new WCHAR[ cchActivationSvr + 1 ];
        if ( NULL == pdasi->wszURL )
        {
            wprintf( L"\nMemory allocation failed for activation URL string.\n" );
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        hr = StringCchCopyW( pdasi->wszURL, 
            cchActivationSvr + 1, 
            wszActivationSvr 
            );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nStringCchCopyW failed. hr = 0x%x.\n", hr );
            goto e_Exit;
        }
        wprintf( L"\nMachine Activation server URL:\n%s\n", pdasi->wszURL );
    }
    else
    {
        //
        // 3(b). Try to find the service location where activation 
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
                wprintf( L"\nMemory allocation failed for activation "\
                         L"URL string.\n" );
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            //
            // Call second time to get the actual service location
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
    // 4. Activate the machine
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
    // 5. Wait for the callback to return
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
    // 5. Clean up and free memory
    //
    if ( NULL != pdasi )
    {
        if ( NULL != pdasi->wszURL )
        {
            delete [] pdasi->wszURL;
        }
        delete pdasi;
    }
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    return hr;
}

//
// This function performs the following actions:
//    1. Validate the parameters
//    2. Create an event for the callback function
//    3. Populate the wszURL member of the DRM_ACTSERV_INFO struct with
//       the user activation server URL.  This value can be from:
//       (a) a command line argument
//       (b) service discovery
//    4. Activate the user
//    5. Wait for the callback to return
//    6. Clean up and free memory
HRESULT __stdcall 
DoUserActivation(
                    DRMHSESSION hClient,
                    __in PWCHAR wszActivationSvr
                    )
{
    HRESULT           hr               = E_FAIL;
    DRM_ACTSERV_INFO *pdasi            = NULL;
    UINT              uiStrLen         = 0;
    size_t            cchActivationSvr = 0;
    DWORD             dwWaitResult;
    DRM_CONTEXT       context;

    context.hEvent = NULL;

    //
    // 1. Validate the parameters
    //
    if ( NULL == hClient )
    {
        wprintf( L"\nThe client session was NULL." );
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // 2. Create an event for the callback function
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call" );
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
    // 3. Copy the activation server URL into the wszURL member
    //    of the DRM_ACTSERV_INFO struct 
    //
    if ( NULL != wszActivationSvr )
    {
        //
        // 3(a). Use the URL provided through the command line argument
        //
        hr = StringCchLengthW( wszActivationSvr, 
            STRSAFE_MAX_CCH, 
            &cchActivationSvr 
            );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nStringCchLengthW failed. hr = 0x%x.\n", hr );
            goto e_Exit;
        }

        //
        // Allocate memory for the URL member of the DRM_ACTSERV_INFO structure
        //
        pdasi->wszURL = new WCHAR[ cchActivationSvr + 1 ];
        if ( NULL == pdasi->wszURL )
        {
            wprintf( L"\nMemory allocation failed for user "\
				     L"activation URL string.\n" );
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        hr = StringCchCopyW( pdasi->wszURL, 
            cchActivationSvr + 1, 
            wszActivationSvr 
            );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nStringCchCopyW failed. hr = 0x%x.\n", hr );
            goto e_Exit;
        }
        wprintf( L"\nUser Activation server URL:\n%s\n", pdasi->wszURL );
    }
    else
    {
        //
        // 3(b). Try to find the service location where user activation 
        //       is available.
        //       The first call is to get -
        //       (a) whether there is a service location and 
        //       (b) if so, the size needed for a buffer to 
        //           hold the URL for the service.
        //
        hr = DRMGetServiceLocation( hClient,
            DRM_SERVICE_TYPE_CERTIFICATION,
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
                wprintf( L"\nMemory allocation failed for user activation "\
                         L"URL string.\n" );
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }

            //
            // Call second time to get the actual service location
            // copied into the URL.
            //
            hr = DRMGetServiceLocation( hClient,
                DRM_SERVICE_TYPE_CERTIFICATION,
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
                L"User Activation server URL:\n%s\n", pdasi->wszURL );
        }
        else
        {
            wprintf( L"\nDRMGetServiceLocation failed. hr = 0x%x. "\
                L"\nPassing NULL server info to DRMActivate.\n", hr );
        }
    }

    //
    // 4. Activate the user
    //
    hr = DRMActivate( hClient,
        DRM_ACTIVATE_GROUPIDENTITY | DRM_ACTIVATE_SILENT,
        0,
        pdasi,
        ( VOID* )&context,
        NULL 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMActivate (DRM_ACTIVATE_GROUPIDENTITY) "\
            L"failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 5. Wait for the callback to return
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

    wprintf( L"\nThe user is now activated.\n" );

e_Exit:

    //
    // 5. Clean up and free memory
    //
    if ( NULL != pdasi )
    {
        if ( NULL != pdasi->wszURL )
        {
            delete [] pdasi->wszURL;
        }
        delete pdasi;
    }
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    return hr;
}

//
// This sample will perform the following actions:
//    1.  Create a client session
//    2.  Determine if the machine needs to be activated
//    3.  Call DoMachineActivation to activate the machine if 
//        it's not activated
//    4.  Determine if the user needs to be activated
//    5.  Call DoUserActivation to activate the user if the user 
//        is not activated
//    6.  Create an event for the callback function
//    7.  If the licensing URL was not passed in via the command line, 
//        find the licensing URL through service discovery
//    8. Acquire the client licensor certificate
//    9.  Wait for the callback to return
//    10. Clean up and free memory
//
int __cdecl 
wmain( 
      int argc, 
      __in_ecount( argc )WCHAR **argv 
      )
{
    HRESULT           hr               = E_FAIL;
    DRMHSESSION       hClient          = NULL;
    HANDLE            hEvent           = NULL;
    PWCHAR            wszActivationSvr = NULL;
    PWCHAR            wszLicensingSvr  = NULL;
    UINT              uiStrLen         = 0;
    PWCHAR            wszUserId        = NULL;
    DWORD             dwWaitResult;
    DRM_CONTEXT       context;

    context.hEvent = NULL;

    if ( FAILED ( ParseCommandLine( argc, 
                                    argv, 
                                    &wszUserId, 
                                    &wszActivationSvr, 
                                    &wszLicensingSvr 
                                    ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample AcquireClientLicensor...\n" );

    //
    // 1. Create a client session
    //
    hr = DRMCreateClientSession( &StatusCallback, 
        0, 
        DRM_DEFAULTGROUPIDTYPE_WINDOWSAUTH, 
        wszUserId, 
        &hClient 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateClientSession failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 2. Call DRMIsActivated to determine if the machine is already activated
    //
    hr = DRMIsActivated( hClient, 
        DRM_ACTIVATE_MACHINE, 
        NULL 
        );
    if ( E_DRM_NEEDS_MACHINE_ACTIVATION == hr )
    {
        //
        // 3.  Call DoMachineActivation to activate the machine if 
		//     it's not activated
		//
        hr = DoMachineActivation( hClient, wszActivationSvr );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDoMachineActivation failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }
    }
    else if ( hr == S_OK )
    {
        wprintf( L"The machine is already activated.\n" );
    }
    else
    {
        wprintf( L"\nDRMIsActivated returned an unexpected failure: 0x%x.  "\
            L"E_DRM_NEEDS_MACHINE_ACTIVATION was expected.\n", hr );
        goto e_Exit;
    }

    //
    // 4. Call DRMIsActivated to determine if the user is 
    //    already activated.  
    //
    hr = DRMIsActivated( hClient, 
        DRM_ACTIVATE_GROUPIDENTITY, 
        NULL 
        );
    if ( SUCCEEDED( hr ) )
    {
        wprintf( L"The user is already activated.\n" );
    }
	else if ( E_DRM_NEEDS_GROUPIDENTITY_ACTIVATION == hr )
	{
		//
		// 5. Call DoUserActivation to activate the user if the user 
		//    is not activated
		//
		hr = DoUserActivation( hClient, wszActivationSvr );
		if ( FAILED( hr ) )
		{
			wprintf( L"\nDoUserActivation failed. hr = 0x%x\n", hr );
			goto e_Exit;
		}
	}
    else if ( E_DRM_NEEDS_GROUPIDENTITY_ACTIVATION != hr )
    {
        wprintf( L"\nDRMIsActivated returned an unexpected failure: 0x%x.  "\
            L"E_DRM_NEEDS_GROUPIDENTITY_ACTIVATION "\
            L"was expected.\n", hr );
        goto e_Exit;
    }

    //
    // 6. Create an event for the callback function.  The StatusCallback
    // function registered in DRMCreateClientSession. This event will be 
    // passed as a void pointer to DRMAcquireLicense.  DRMAcquireLicense 
    // simply passes back this pointer to the StatusCallback callback 
    // function which knows that it is an event and will thus signal it 
    // when completed.
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call." );
        goto e_Exit;
    }

    //
    // 7. If the licensing URL was not passed in via the command line, 
    //    find the licensing URL through service discovery
    //
    if ( NULL == wszLicensingSvr )
    {
        //       The first call is to get -
        //       (a) whether there is a service location and 
        //       (b) if so, the size needed for a buffer to 
        //           hold the URL for the service.
        //
        hr = DRMGetServiceLocation( hClient, 
                                    DRM_SERVICE_TYPE_CLIENTLICENSOR, 
                                    DRM_SERVICE_LOCATION_ENTERPRISE,
                                    NULL,
                                    &uiStrLen,
                                    NULL
                                    );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMGetServiceLocation failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }

        wszLicensingSvr = new WCHAR[ uiStrLen ];
        if ( NULL == wszLicensingSvr )
        {
            wprintf( L"\nMemory allocation failed for licensing URL string.\n" );
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        hr = DRMGetServiceLocation( hClient,
                                    DRM_SERVICE_TYPE_CLIENTLICENSOR,
                                    DRM_SERVICE_LOCATION_ENTERPRISE,
                                    NULL,
                                    &uiStrLen,
                                    wszLicensingSvr
                                    );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMGetServiceLocation failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }
    }

    wprintf( L"Licensing URL: %s\n", wszLicensingSvr );

    //
    // 8. Acquire the client licensor certificate
    //
    hr = DRMAcquireLicense( hClient, 
                            0, 
                            NULL, 
                            NULL, 
                            NULL, 
                            wszLicensingSvr, 
                            ( VOID* )&context 
                            );
    if ( FAILED( hr ) )
    {
        wprintf( L"DRMAcquireLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 9. Wait for the callback to return
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
        hr = context.hr;
        wprintf( L"\nThe callback function returned a failure "\
            L"code. hr = 0x%x\n", context.hr );
        goto e_Exit;
    }

    wprintf( L"\nThe client licensor certificate has been acquired.\n" );

e_Exit:
    //
    // 10. Clean up and free memory
    //
    if ( NULL != wszActivationSvr )
    {
        delete [] wszActivationSvr;
    }
    if ( NULL != wszLicensingSvr )
    {
        delete [] wszLicensingSvr;
    }
    if ( NULL != wszUserId )
    {
        delete [] wszUserId;
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
        wprintf( L"\nAcquireClientLicensor failed. hr = 0x%x\n", hr );
    }
    return hr;
}