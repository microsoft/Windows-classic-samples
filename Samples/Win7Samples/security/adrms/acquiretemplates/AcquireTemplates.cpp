/*=====================================================================
File:      acquiretemplates.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample shows how to download and enumerate templates.  It 
// takes an optional licensing server URL as input.
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
    wprintf( L"\n  AcquireTemplates -U UserID [-L LicensingSvr]\n" );
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
    HRESULT hr = S_OK;
    size_t uiUserIDLength = 0;
    size_t uiLicSvrUrlLength = 0;

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
    case DRM_MSG_ACQUIRE_ISSUANCE_LICENSE_TEMPLATE:
        wprintf( L"\nCallback status msg = DRM_MSG_ACQUIRE_ISSUANCE_LICENSE_TEMPLATE " );
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
//    1. Create an empty issuance license from the supplied template.  
//    2. Query the name and description from the empty issuance license.  
//       These will match the name and description found in the template.
//    3. Print the name, description, and raw template XML 
HRESULT __stdcall 
PrintTemplate(      UINT    uiIndex,
                    BOOL    fSharedFlag,
              __in  PWCHAR  wszTemplate
             )
{
    DRMPUBHANDLE    hIssuanceLicense    = NULL;
    HRESULT         hr;
    PWCHAR          wszDescription      = NULL;
    PWCHAR          wszName             = NULL;
    UINT            uiCid;
    UINT            uDescriptionLength;
    UINT            uNameLength;

    //
    // 1. Create an empty issuance license from the supplied template.  
    //
    hr = DRMCreateIssuanceLicense( NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   wszTemplate,
                                   NULL,
                                   &hIssuanceLicense 
                                   );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    //  2. Query the name and description from the empty issuance license.  
    //     These will match the name and description found in the template.
    //
    hr = DRMGetNameAndDescription( hIssuanceLicense,
                                   0,
                                   &uiCid,
                                   &uNameLength,
                                   NULL,
                                   &uDescriptionLength,
                                   NULL
                                   );
    if ( FAILED ( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszName = new WCHAR[ uNameLength ];
    if ( NULL == wszName )
    {
        wprintf( L"\nMemory allocation failed for template name.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    wszDescription = new WCHAR[ uDescriptionLength ];
    if ( NULL == wszDescription )
    {
        wprintf( L"\nMemory allocation failed for template description.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = DRMGetNameAndDescription( hIssuanceLicense,
                                   0,
                                   &uiCid,
                                   &uNameLength,
                                   wszName,
                                   &uDescriptionLength,
                                   wszDescription
                                   );
    if ( FAILED ( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    //    3. Print the name, description, and raw template XML 
    //
    wprintf(L"\n *** TEMPLATE # %d *** \n\n", uiIndex);
    wprintf(L"\tShared:  %s\n", fSharedFlag ? L"TRUE" : L"FALSE");
    wprintf(L"\tName:   %s\n", wszName);
    wprintf(L"\tDescription:   %s\n", wszDescription);
    wprintf(L"\tTemplate:  %s\n", wszTemplate);

e_Exit:
    if ( NULL != hIssuanceLicense )
    {
        DRMClosePubHandle( hIssuanceLicense );
    }
    if ( NULL != wszName )
    {
        delete [] wszName;
    }
    if ( NULL != wszDescription )
    {
        delete [] wszDescription;
    }
    return hr;
}

//
// This sample will perform the following actions:
//    1.  Create a client session
//    2.  Create an event for the callback function
//    3.  If the licensing URL was not passed in via the command line, 
//        find the licensing URL through service discovery
//    4.  Download the templates into the local store
//    5.  Wait for the callback to return
//    6. Enumerate templates from the local store
//    7. Clean up and free memory
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
    PWCHAR            wszLicensingSvr  = NULL;
    BOOL              fSharedFlag;
    UINT              uCertificateDataLen;
    PWCHAR            wszCertificateData = NULL;
    UINT              uiIndex;
    UINT              uiStrLen         = 0;
    PWCHAR            wszUserId        = NULL;
    DWORD             dwWaitResult;
    DRM_CONTEXT       context;

    context.hEvent = NULL;

    if ( FAILED ( ParseCommandLine( argc, 
                                    argv, 
                                    &wszUserId, 
                                    &wszLicensingSvr 
                                    ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample AcquireTemplates...\n" );

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
    // 2. Create an event for the callback function.  The StatusCallback
    // function registered in DRMCreateClientSession. This event will be 
    // passed as a void pointer to DRMAcquireIssuanceLicenseTemplate.  
    // DRMAcquireIssuanceLicenseTemplate simply passes back this pointer 
    // to the StatusCallback callback function which knows that it is an
    // event and will thus signal it when completed.
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call." );
        goto e_Exit;
    }

    //
    // 3. If the licensing URL was not passed in via the command line, 
    //    find the licensing URL through service discovery. 
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
    // 4. Download the current templates to our license store
    //    *** NOTE: templates will be periodically downloaded to your 
    //              local store automatically by the operating system, 
    //              so calling this function explicitly is not 
    //              recommended for most applications.
    hr = DRMAcquireIssuanceLicenseTemplate( hClient,
                                            DRM_AILT_OBTAIN_ALL,
                                            0,
                                            0,
                                            NULL,
                                            wszLicensingSvr,
                                            ( VOID* )&context 
                                            );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMAcquireIssuanceLicenseTemplate failed. hr = 0x%x\n", hr );
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
        hr = context.hr;
        wprintf( L"\nThe callback function returned a failure "\
            L"code. hr = 0x%x\n", context.hr );
        goto e_Exit;
    }

    //
    // 6.  Enumerate templates from the local store
    //
    uiIndex = 0;

    while (TRUE)
    {
        hr = DRMEnumerateLicense( hClient,
                                  DRM_EL_ISSUANCELICENSE_TEMPLATE,
                                  uiIndex, 
                                  &fSharedFlag, 
                                  &uCertificateDataLen,
                                  NULL
                                  );
        if (FAILED(hr))
        {
            if (E_DRM_NO_MORE_DATA == hr)
            {
                //
                // Done enumerating
                //
                hr = S_OK;
                break;
            }

            wprintf( L"\nDRMEnumerateLicense failed. hr = 0x%x\n", hr );       
            goto e_Exit;
        }

        wszCertificateData = new WCHAR[ uCertificateDataLen ];
        if ( NULL == wszLicensingSvr )
        {
            wprintf( L"\nMemory allocation failed for local template string.\n" );
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        hr = DRMEnumerateLicense( hClient,
                                  DRM_EL_ISSUANCELICENSE_TEMPLATE,
                                  uiIndex, 
                                  &fSharedFlag, 
                                  &uCertificateDataLen,
                                  wszCertificateData
                                  );
        if (FAILED(hr)) 
        {
            wprintf( L"\nDRMEnumerateLicense failed. hr = 0x%x\n", hr );       
            goto e_Exit;
        }

        PrintTemplate(uiIndex, fSharedFlag, wszCertificateData);

        delete [] wszCertificateData;
        wszCertificateData = NULL;

        uiIndex++;
    }


    wprintf( L"\nThe templates have been downloaded.\n" );

e_Exit:
    //
    // 11. Clean up and free memory
    //
    if ( NULL != wszLicensingSvr )
    {
        delete [] wszLicensingSvr;
    }
    if ( NULL != wszUserId )
    {
        delete [] wszUserId;
    }
    if ( NULL != wszCertificateData )
    {
        delete [] wszCertificateData;
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
        wprintf( L"\nAcquireTemplates failed. hr = 0x%x\n", hr );
    }
    return hr;
}