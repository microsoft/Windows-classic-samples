/*=====================================================================
File:      OnlinePublishing.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample demonstrates online publishing. It takes an optional 
// licensing server URL as input.  It will use Service Discovery to 
// find the server URL if it is not provided.  The sample requires that 
// a UserID be provided as input.  See the comments at the beginning of 
// wmain() for a more detailed description.
//

#include <stdio.h>
#include <Wtypes.h>
#include <strsafe.h>
#include <objbase.h>

#include "msdrm.h"
#include "msdrmdefs.h"
#include "msdrmerror.h"

//
// Time to wait for "downloads" to complete
//
static const DWORD DW_WAIT_TIME_SECONDS = 60 * 1000;

//
// Length of a GUID string
//
static const UINT GUID_STRING_LENGTH = 128;

//
// struct to hold the callback information
//
typedef struct Drm_Context
{
    HANDLE  hEvent;
    HRESULT hr;
    PWSTR   wszData;
} DRM_CONTEXT, *PDRM_CONTEXT;

//
// Print the correct usage of this application
//
void 
PrintUsage()
{
    wprintf( L"Usage:\n" );
    wprintf( L"\n  OnlinePublishing -U UserID [-L LicensingSvr]\n" );
    wprintf( L"    -U: specifies the UserID.\n" );
    wprintf( L"        example: user@yourdomain.com\n" );
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
                 __deref_out PWCHAR *pwszLicensingSvr
                 )
{
    HRESULT hr                = S_OK;
    size_t  uiUserIDLength    = 0;
    size_t  uiLicSvrUrlLength = 0;

    //
    // Initialize parameters
    //
    *pwszUserID = NULL;

    //
    // Check input parameters.
    //
    if ( ( 3 != argc ) && ( 5 != argc ) )
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
    size_t uiSignedILLength = 0;
    PDRM_CONTEXT pContext = ( PDRM_CONTEXT )pvContext;

    if ( pContext )
    {
        pContext->hr = hr;
    }
    else
    {
        return E_FAIL;
    }

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
	case DRM_MSG_SIGN_ISSUANCE_LICENSE:
		wprintf( L"\nCallback status msg = DRM_MSG_SIGN_ISSUANCE_LICENSE " );
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
        if ( ( msg == DRM_MSG_SIGN_ISSUANCE_LICENSE ) && pvParam )
        {
            if ( NULL != pvParam )
            {
                //
                // Retrieve the length of the signed issuance license
                //
                hr = StringCchLengthW( ( PWSTR )pvParam, 
                                       STRSAFE_MAX_CCH, 
                                       &uiSignedILLength
                                       );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                    break;
                }
                //
                // Allocate memory for the signed issuance license
                //
                pContext->wszData = new WCHAR[ uiSignedILLength + 1 ];
                if( NULL == pContext->wszData ) 
                {
                    wprintf( L"Failed to allocate memory "\
                        L"for the signed issuance license.\n" );
                    hr = E_OUTOFMEMORY;
                    break;
                }
                //
                // Copy the signed issuance license into the 
                // pContext->wszData buffer
                //
                hr = StringCchCopyW( ( wchar_t* )pContext->wszData, 
                    uiSignedILLength + 1 , 
                    ( PWSTR )pvParam 
                    );
                if ( FAILED( hr ) )
                {
                    wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
            }
        }
		if ( pContext->hEvent )
		{
			SetEvent( ( HANDLE )pContext->hEvent );
		}
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
    case E_DRM_REQUEST_DENIED:
        wprintf( L"Callback hr = E_DRM_REQUEST_DENIED\n" );
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
// This sample will perform the following actions:
//    1.  Create a client session
//    2.  Create an issuance license, user, and right.  Add the right/user
//        pair to the issuance license.
//    3.  Set the metadata in the issuance license
//    4.  Create an event for the callback function.
//    5.  If the licensing URL was not passed in via the command line, 
//        find the licensing URL through service discovery
//    6.  Sign the issuance license online
//    7.  Wait for the callback to return
//    8.  Clean up and free memory
//
int __cdecl 
wmain( 
      int argc, 
      __in_ecount( argc )WCHAR **argv 
      )
{
    HRESULT           hr               = E_FAIL;
    DRMHSESSION       hClient          = NULL;
    PWCHAR            wszLicensingSvr  = NULL;
    UINT              uiStrLen         = 0;
    PWSTR             wszUserId        = NULL;
    DWORD             dwWaitResult;
    DRM_CONTEXT       context;
    SYSTEMTIME        stTimeFrom, stTimeUntil;
    DRMPUBHANDLE      hIssuanceLicense = NULL;
    DRMPUBHANDLE      hUser            = NULL;
    DRMPUBHANDLE      hRight           = NULL;
    GUID              guid;
    PWSTR             wszGUID          = NULL;
    UINT              uiGUIDLength;

    context.hEvent = NULL;
    context.wszData = NULL;

    if ( FAILED ( ParseCommandLine( argc, argv, &wszUserId, &wszLicensingSvr ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample OnlinePublishing...\n" );

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
    // Get the system time for the starting and ending validity times
    // in the unsigned issuance license
    //
    GetSystemTime( &stTimeFrom );
    GetSystemTime( &stTimeUntil );
    
    FILETIME ft = { 0 };
    SystemTimeToFileTime(&stTimeUntil, &ft);

    ULARGE_INTEGER u = { 0 };
    memcpy(&u, &ft, sizeof(u));
    u.QuadPart += 90 * 24 * 60 * 60 * 10000000LLU;  // 90 days
    memcpy(&ft, &u, sizeof(ft));

    FileTimeToSystemTime(&ft, &stTimeUntil);

    // 
    // 2. Create an issuance license, user, and right.  Add the right/user
    //    pair to the issuance license.
    //
    hr = DRMCreateIssuanceLicense( &stTimeFrom,
        &stTimeUntil,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &hIssuanceLicense 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    hr = DRMCreateUser( wszUserId, NULL, L"Unspecified", &hUser );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateUser failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    hr = DRMCreateRight( L"EDIT", &stTimeFrom, &stTimeUntil, 
                         0, NULL, NULL, &hRight );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateRight failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    hr = DRMAddRightWithUser( hIssuanceLicense, hRight, hUser );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMAddRightWithUser failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Create a GUID to use as a unique content ID
    // in the issuance license
    //
    hr = CoCreateGuid( &guid );
    if ( FAILED( hr ) )
    {
        wprintf( L"CoCreateGuid failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszGUID = new WCHAR[ GUID_STRING_LENGTH ];
    if ( NULL == wszGUID )
    {
        wprintf( L"Failed to allocate memory for wszGUID\n" );
        goto e_Exit;
    }

    uiGUIDLength = StringFromGUID2( guid, wszGUID, GUID_STRING_LENGTH );
    if ( 0 == uiGUIDLength )
    {
        wprintf( L"StringFromGUID2 failed.\n" );
        hr = E_FAIL;
        goto e_Exit;
    }

    //
    // 3.  Set the metadata in the issuance license
    //
    hr = DRMSetMetaData( hIssuanceLicense, 
                         wszGUID, 
                         L"MS-GUID", 
                         NULL, 
                         NULL, 
                         NULL, 
                         NULL 
                         );
    if ( FAILED( hr ) )
    {
        wprintf( L"DRMSetMetaData failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 4. Create an event for the callback function.  The StatusCallback
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
    // 5. If the licensing URL was not passed in via the command line, 
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
                                    DRM_SERVICE_TYPE_PUBLISHING, 
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
                                    DRM_SERVICE_TYPE_PUBLISHING,
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

    wprintf( L"\nLicensing URL: %s\n", wszLicensingSvr );

    //
    // 6. Sign the issuance license online
    //
    hr = DRMGetSignedIssuanceLicense( NULL, 
                                      hIssuanceLicense,
                                      DRM_SIGN_ONLINE | DRM_AUTO_GENERATE_KEY,
                                      NULL,
                                      0,
                                      L"AES",
                                      NULL,
                                      &StatusCallback,
                                      wszLicensingSvr,
                                      ( void* )&context
                                      );
    if ( FAILED( hr ) )
    {
        wprintf( L"DRMGetSignedIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 7. Wait for the callback to return
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
        wprintf( L"\nThe callback function returned failure "\
            L"code. hr = 0x%x\n", context.hr );
        goto e_Exit;
    }

    wprintf( L"\nThe signed issuance license is: \n\n%s\n", context.wszData );

    wprintf( L"\nDRMGetSignedIssuanceLicense succeeded.\n" );

e_Exit:
    //
    // 8. Clean up and free memory
    //
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
    if ( NULL != context.wszData )
    {
        delete [] context.wszData;
    }
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    if ( FAILED( hr ) )
    {
        wprintf( L"\nOnlinePublishing failed. hr = 0x%x\n", hr );
    }
    return hr;
}