/*=====================================================================
File:      OfflinePublishing.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample demonstrates offline publishing.  It acquires the appropriate
// licenses and certificates, initializes the environment, and signs the 
// publishing license offline.  See the comments at the beginning of wmain() 
// for a more detailed description.
//

#include <stdio.h>
#include <Wtypes.h>
#include <strsafe.h>
#include <objbase.h>

#include "msdrm.h"
#include "msdrmdefs.h"
#include "msdrmerror.h"

//
// Values to put into the unsigned issuance license
//
#define APP_SPECIFIC_DATA_NAME L"AppSpecificName"
#define APP_SPECIFIC_DATA_VALUE L"AppSpecificValue"
#define INTERVAL_TIME_DAYS 30
#define SKU_ID L"SKUId"
#define SKU_ID_TYPE L"SKUIdType"
#define CONTENT_TYPE L"ContentType"
#define CONTENT_NAME L"ContentName"
#define NAME_AND_DESCRIPTION_NAME L"Name"
#define NAME_AND_DESCRIPTION_DESCRIPTION L"Description"
#define USAGE_POLICY_APPLICATION_NAME L"ApplicationName"
#define USAGE_POLICY_MIN_VERSION L"1.0.0.0"
#define USAGE_POLICY_MAX_VERSION L"3.0.0.0"
#define RIGHT_NAME L"EDIT"

//
// Time to wait for "downloads" to complete
//
static const DWORD DW_WAIT_TIME_SECONDS = 60 * 1000;  

//
// Length of a GUID string
//
static const UINT GUID_STRING_LENGTH = 128;


//
// Unicode Byte Order Mark Length
//
static const UINT UTF16_BOM_LENGTH = 2;

//
// Struct to hold the callback information
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
    wprintf( L"\n  OfflinePublishing -U UserID -M ManifestFile "\
             L"[-A ActivationSvr] [-L LicensingSvr]\n" );
    wprintf( L"    -U: specifies the UserID.\n" );
    wprintf( L"        example: user@yourdomain.com\n" );
    wprintf( L"    -M: specifies the manifest file to use.\n" );
    wprintf( L"        example: manifest.xml\n" );
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
                 __deref_out PWCHAR *pwszLicensingSvr,
                 __deref_out_opt PWCHAR *pwszManifestFile
                 )
{
    HRESULT hr                   = S_OK;
    size_t  uiUserIDLength       = 0;
    size_t  uiActSvrUrlLength    = 0;
    size_t  uiLicSvrUrlLength    = 0;
    size_t  uiManifestFileLength = 0;

    //
    // Initialize parameters
    //
    *pwszUserID = NULL;
    *pwszManifestFile = NULL;

    //
    // Check input parameters.
    //
    if ( ( 5 != argc ) && ( 7 != argc ) && ( 9 != argc ) )
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
                wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the user ID
            //
            *pwszUserID = new WCHAR[ uiUserIDLength + 1 ];
            if ( NULL == *pwszUserID ) 
            {
                wprintf( L"\nFailed to allocate memory for pwszUserID.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the user ID into the pwszUserID buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszUserID, 
                uiUserIDLength + 1 , 
                argv[ i + 1 ] 
                );
                if ( FAILED( hr ) )
                {
                    wprintf( L"\nStringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        case 'A':
            if ( ( _wcsnicmp( argv[ i + 1 ], L"http://", 7 ) != 0 ) && 
                ( _wcsnicmp( argv[ i + 1 ], L"https://", 8 ) != 0 ) )
            {
                wprintf( L"\nInvalid activation URL provided.\n" );
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
                wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the URL
            //
            *pwszActivationSvr = new WCHAR[ uiActSvrUrlLength + 1 ];
            if( NULL == *pwszActivationSvr ) 
            {
                wprintf( L"\nFailed to allocate memory "\
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
                    wprintf( L"\nStringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        case 'L':
            if ( ( _wcsnicmp( argv[ i + 1 ], L"http://", 7 ) != 0 ) && 
                ( _wcsnicmp( argv[ i + 1 ], L"https://", 8 ) != 0 ) )
            {
                wprintf( L"\nInvalid licensing URL provided.\n" );
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
                wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the URL
            //
            *pwszLicensingSvr = new WCHAR[ uiLicSvrUrlLength + 1 ];
            if( NULL == *pwszLicensingSvr ) 
            {
                wprintf( L"\nFailed to allocate memory "\
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
                    wprintf( L"\nStringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
                i++;
                break;
        case 'M':
            if ( ( wcsstr( argv[ i + 1 ], L".xml" ) == NULL ) && 
                ( wcsstr( argv[ i + 1 ], L".XML" ) == NULL ) )
            {
                wprintf( L"\nInvalid manifest file name provided.\n" );
                hr = E_INVALIDARG;
                break;
            }
            //
            // Retrieve the length of the manifest name
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiManifestFileLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the manifest file name
            //
            *pwszManifestFile = new WCHAR[ uiManifestFileLength + 1 ];
            if( NULL == *pwszManifestFile ) 
            {
                wprintf( L"\nFailed to allocate memory "\
                    L"for pwszActivationSvr.\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the manifest name into the pwszManifestFile buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszManifestFile, 
                uiManifestFileLength + 1 , 
                argv[ i + 1 ] 
            );
            if ( FAILED( hr ) )
            {
                wprintf( L"\nStringCchCopyW failed.  hr = 0x%x\n", hr );
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
        wprintf( L"\nA user ID value is required.\n" );
        hr = E_INVALIDARG;
    }
    if ( NULL == *pwszManifestFile )
    {
        wprintf( L"\nA manifest file is required.\n" );
        hr = E_INVALIDARG;
    }
    return hr;
}

//
// Copy the contents of the file into a string buffer
//
HRESULT
ReadFileToWideString(
                     __in PWSTR szFileName,
                     __deref_out PWCHAR *pwszManifestString
                     )
{
    HRESULT                    hr            = S_OK;
    HANDLE                     hFile         = INVALID_HANDLE_VALUE;
    DWORD                     dwBytesRead   = 0;
    UINT                          uiUnicode      = 0;
    BYTE                          bBOMValue[UTF16_BOM_LENGTH];

	
    BY_HANDLE_FILE_INFORMATION fileInfo;

    //
    // Validate the parameters
    //
    if ( ( NULL == szFileName ) ||
         ( FAILED( StringCchLengthW( szFileName, MAX_PATH, NULL ) ) ) ||
         ( NULL == pwszManifestString ) )
    {
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // Get a handle to the file
    //
    hFile = CreateFileW( szFileName, 
                         GENERIC_READ, 
                         0, 
                         NULL, 
                         OPEN_EXISTING, 
                         0, 
                         NULL 
                         );
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        wprintf( L"\nCreateFileW failed. hFile == INVALID_HANDLE_VALUE\n" );
        hr = HRESULT_FROM_WIN32( GetLastError() ) ;
        goto e_Exit;
    }

    if ( 0 == GetFileInformationByHandle( hFile, &fileInfo ) )
    {
        wprintf( L"\nGetFileInformationByHandle failed.\n" );
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto e_Exit;
    }

    //
    // Check for files that are too long and unsigned integer overflow
    //
    if ( ( 0 != fileInfo.nFileSizeHigh ) || 
         ( fileInfo.nFileSizeLow + sizeof( WCHAR ) < fileInfo.nFileSizeLow ) )
    {
        wprintf(L"\nFile too long.\n");
        hr = E_FAIL;
        goto e_Exit;
    }


   memset(bBOMValue, 0, sizeof(bBOMValue) );
	

   //
   // First check to see if the file is in UTF-16 format
   //
   if (0 == ReadFile( hFile, 
   	                         (LPVOID)bBOMValue,
   	                         UTF16_BOM_LENGTH,
                                &dwBytesRead,
                                0
                                ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto e_Exit;
    }
   				     
    if( (bBOMValue[0] == 0xFF ) && ( bBOMValue[1] == 0xFE ) ) {

	 uiUnicode = UTF16_BOM_LENGTH;
	 
    } else {

	 //
    	 //Reset the pointer if this file is not in UTF-16 format
    	 //
        if(0 == SetFilePointer( hFile,
                                          0,
                                          NULL,
                                          FILE_BEGIN
                                          ) )
        {
             hr = HRESULT_FROM_WIN32( GetLastError() );
	      goto e_Exit;
        }
						
    }

    *pwszManifestString = new WCHAR[ fileInfo.nFileSizeLow + sizeof(WCHAR) ];
    if ( NULL == *pwszManifestString )
    {
        wprintf( L"\nMemory allocation for pwszManifestString failed.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }


    memset(*pwszManifestString, 0, (fileInfo.nFileSizeLow + sizeof(WCHAR) ) * sizeof(WCHAR) );	


    //
    // Put the contents of the file into pwszManifestString
    //
    if(0 == ReadFile( hFile,
			           *pwszManifestString,
			           fileInfo.nFileSizeLow - uiUnicode,
			           &dwBytesRead,
			           0
			           ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto e_Exit;
    }

  

e_Exit:
    if ( INVALID_HANDLE_VALUE != hFile )
    {
        CloseHandle( hFile );
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
    size_t       uiSignedILLength = 0;
    PDRM_CONTEXT pContext         = ( PDRM_CONTEXT )pvContext;
    
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
    case DRM_MSG_ACTIVATE_MACHINE:
        wprintf( L"\nCallback status msg = DRM_MSG_ACTIVATE_MACHINE " );
        break;
    case DRM_MSG_ACTIVATE_GROUPIDENTITY:
        wprintf( L"\nCallback status msg = DRM_MSG_ACTIVATE_GROUPIDENTITY " );
        break;
	case DRM_MSG_ACQUIRE_CLIENTLICENSOR:
		wprintf( L"\nCallback status msg = DRM_MSG_ACQUIRE_CLIENTLICENSOR " );
		break;
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
                    wprintf( L"\nStringCchLengthW failed.  hr = 0x%x\n", hr );
                    break;
                }
                //
                // Allocate memory for the signed issuance license
                //
                pContext->wszData = new WCHAR[ uiSignedILLength + 1 ];
                if( NULL == pContext->wszData ) 
                {
                    wprintf( L"\nFailed to allocate memory "\
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
                    wprintf( L"\nStringCchCopyW failed.  hr = 0x%x\n", hr );
                    break;
                }
            }
		}
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
        wprintf( L"\nThe client session was NULL.\n" );
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // 2. Create an event for the callback function
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call\n" );
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
        // Allocate memory for the URL member of 
        // the DRM_ACTSERV_INFO structure
        //
        pdasi->wszURL = new WCHAR[ cchActivationSvr + 1 ];
        if ( NULL == pdasi->wszURL )
        {
            wprintf( L"\nMemory allocation failed for the "\
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
        wprintf( L"\nWaitForSingleObject timed out.\n" );
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
    // 6. Clean up and free memory
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
        wprintf( L"\nThe client session was NULL.\n" );
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // 2. Create an event for the callback function
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call\n" );
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
        // Allocate memory for the URL member of 
        // the DRM_ACTSERV_INFO structure
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
        wprintf( L"\nWaitForSingleObject timed out.\n" );
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
    // 6. Clean up and free memory
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
//    3. Populate the wszLicensingSvr parameter.  This value can
//       be from:
//       (a) a command line argument
//       (b) service discovery
//    4. Acquire the client licensor certificate
//    5. Wait for the callback to return
//    6. Enumerate the client licensor certificate
//    7. Clean up and free memory
//
HRESULT
DoAcquireClientLicensorCertificate( 
                                   DRMHSESSION hClient,
                                   __in PWCHAR wszLicensingSvr, 
                                   __in PWCHAR wszUserId, 
                                   __deref_out PWCHAR *wszClientLicensorCert 
                                   )
{
    HRESULT     hr                         = E_FAIL;
    UINT        uiStrLen                   = 0;
    UINT        uiClientLicensorCertLength = 0;
    BOOL        fShared                    = false;
    DRM_CONTEXT context;
    DWORD       dwWaitResult;

    context.hEvent = NULL;

    //
    // 1. Validate the parameters
    //
    if ( ( NULL == hClient ) || ( NULL == wszUserId ) )
    {
        wprintf( L"\nInvalid parameter.\n" );
        hr = E_INVALIDARG;
        goto e_Exit;
    }

    //
    // 2. Create an event for the callback function.  This event will be 
    // passed as a void pointer to DRMAcquireLicense.  DRMAcquireLicense 
    // simply passes back this pointer to the StatusCallback callback 
    // function which knows that it is an event and will thus signal it 
    // when completed.
    //
    if ( NULL == ( context.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ) ) )
    {
        wprintf( L"\ncontext.hEvent was NULL after the CreateEvent call.\n" );
        goto e_Exit;
    }

    //
    // 3. If the licensing URL was not passed in via the command line, 
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
            wprintf( L"\nDRMGetServiceLocation failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }

        wszLicensingSvr = new WCHAR[ uiStrLen ];
        if ( NULL == wszLicensingSvr )
        {
            wprintf( L"\nMemory allocation failed for the "\
                     L"licensing URL string.\n" );
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
            wprintf( L"\nDRMGetServiceLocation failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }
    }

    wprintf( L"\nLicensing server URL:\n%s\n", wszLicensingSvr );

    //
    // 4. Acquire the client licensor certificate
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
        wprintf( L"\nDRMAcquireLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 5. Wait for the callback to return
    //
    dwWaitResult = WaitForSingleObject( context.hEvent, DW_WAIT_TIME_SECONDS );
    if ( WAIT_TIMEOUT == dwWaitResult )
    {
        wprintf( L"\nWaitForSingleObject timed out.\n" );
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

    wprintf( L"\nA client licensor certificate has been acquired.\n" );

    // 
    // 6. Enumerate the client licensor certificate to return it
    //
    hr = DRMEnumerateLicense( hClient, 
                              DRM_EL_SPECIFIED_CLIENTLICENSOR, 
                              0, 
                              &fShared, 
                              &uiClientLicensorCertLength,
                              NULL
                              );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMEnumerateLicense (DRM_EL_SPECIFIED_CLIENTLICENSOR) "\
                 L"failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    *wszClientLicensorCert = new WCHAR[ uiClientLicensorCertLength ];
    if ( NULL == *wszClientLicensorCert )
    {
        wprintf( L"\nMemory allocation for wszClientLicensorCert failed.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = DRMEnumerateLicense( hClient, 
        DRM_EL_SPECIFIED_CLIENTLICENSOR,
        0,
        &fShared,
        &uiClientLicensorCertLength,
        *wszClientLicensorCert
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMEnumerateLicense(DRM_EL_SPECIFIED_CLIENTLICENSOR)"\
            L" failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

e_Exit:
    //
    //    7. Clean up and free memory
    //
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    return hr;
}

//
// This sample will perform the following actions:
//    1.  Create a client session
//    2.  Determine if the machine is already activated
//    3.  Call DoMachineActivation if the machine needs to be activated
//    4.  Enumerate the machine certificate
//    5.  Determine if the user is already activated
//    6.  Call DoUserActivation if the user needs to be activated
//    7.  Enumerate the client licensor certificates to determine if one
//        has already been acquired
//    8.  If there are no client licensor certificates, call 
//        DoAcquireClientLicensorCertificate to acquire one
//    9.  Read the manifest file into a buffer
//    10. Get the security provider
//    11. Initialize an environment
//    12. Create an unsigned issuance license from scratch
//    13. Set the metadata, application specific data, interval time,
//        and usage policy in the unsigned issuance license
//    14. Create a user and right and add those to the 
//        unsigned issuance license
//    15. Sign the unsigned issuance license offline
//    16. Wait for the callback to return
//    17.  Clean up and free memory
//
int __cdecl 
wmain( 
      int argc, 
      __in_ecount( argc )WCHAR **argv 
      )
{
    HRESULT                   hr                           = E_FAIL;
    DRMHSESSION               hClient                      = NULL;
    DRMPUBHANDLE              hIssuanceLicense             = NULL;
    DRMPUBHANDLE              hUser                        = NULL;
    DRMPUBHANDLE              hRight                       = NULL;
    DRMENVHANDLE              hEnv                         = NULL;
    DRMHANDLE                 hLib                         = NULL;
    GUID                      guid;
    BOOL                      fShared                      = false;
    SYSTEMTIME                stTimeFrom, stTimeUntil;
    PWSTR                     wszGUID                      = NULL;
    PWCHAR                    wszUserId                    = NULL;
    PWCHAR                    wszActivationSvr             = NULL;
    PWCHAR                    wszLicensingSvr              = NULL;
    PWCHAR                    wszManifestFileName          = NULL;
    PWCHAR                    wszManifest                  = NULL;
    PWCHAR                    wszClientLicensorCert        = NULL;
    PWCHAR                    wszSecurityProviderType      = NULL;
    PWCHAR                    wszSecurityProviderPath      = NULL;
    PWCHAR                    wszMachineCertificate        = NULL;
    UINT                      uiMachineCertLength          = 0;
    UINT                      uiSecurityProviderTypeLength = 0;
    UINT                      uiSecurityProviderPathLength = 0;
    UINT                      uiGUIDLength                 = 0;
    UINT                      uiClientLicensorCertLength   = 0;
    DRM_CONTEXT               context;
    DWORD                     dwWaitResult;

    context.hEvent = NULL;
    context.wszData = NULL;

    if ( FAILED ( ParseCommandLine( argc, 
                                    argv, 
                                    &wszUserId, 
                                    &wszActivationSvr, 
                                    &wszLicensingSvr, 
                                    &wszManifestFileName
                                    ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample OfflinePublishing...\n" );

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
        wprintf( L"\nThe machine is already activated.\n" );
    }
    else
    {
        wprintf( L"\nDRMIsActivated returned an unexpected failure: 0x%x.  "\
            L"E_DRM_NEEDS_MACHINE_ACTIVATION was expected.\n", hr );
        goto e_Exit;
    }

    //
    // 4. Enumerate the machine certificate
    //
    hr = DRMEnumerateLicense( hClient, 
                              DRM_EL_MACHINE, 
                              0, 
                              &fShared, 
                              &uiMachineCertLength, 
                              NULL 
                              );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMEnumerateLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszMachineCertificate = new WCHAR[ uiMachineCertLength ];
    if ( NULL == wszMachineCertificate )
    {
        wprintf( L"\nMemory allocation failed for wszMachineCertificate\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = DRMEnumerateLicense( hClient, 
                              DRM_EL_MACHINE, 
                              0, 
                              &fShared, 
                              &uiMachineCertLength, 
                              wszMachineCertificate 
                              );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMEnumerateLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 5. Call DRMIsActivated to determine if the user is 
    //    already activated.  
    //
    hr = DRMIsActivated( hClient, 
        DRM_ACTIVATE_GROUPIDENTITY, 
        NULL 
        );
    if ( SUCCEEDED( hr ) )
    {
        wprintf( L"\nThe user is already activated.\n" );
    }
	else if ( E_DRM_NEEDS_GROUPIDENTITY_ACTIVATION == hr )
	{
		//
		// 6. Call DoUserActivation to activate the user if the user 
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
    // 7. Enumerate the client licensor certificates to determine if one
    //    has already been acquired.  If not, get a client licensor cert.
    //
    hr = DRMEnumerateLicense( hClient, 
                              DRM_EL_SPECIFIED_CLIENTLICENSOR, 
                              0, 
                              &fShared, 
                              &uiClientLicensorCertLength,
                              NULL
                              );
    if ( FAILED( hr ) && ( E_DRM_NO_MORE_DATA != hr ) )
    {
        wprintf( L"\nDRMEnumerateLicense (DRM_EL_SPECIFIED_CLIENTLICENSOR) "\
                 L"failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }
    else if ( E_DRM_NO_MORE_DATA == hr )
    {
        //    8.  If there are no client licensor certificates, call 
        //        DoAcquireClientLicensorCertificate to acquire one
        hr = DoAcquireClientLicensorCertificate( hClient,
                                                 wszLicensingSvr, 
                                                 wszUserId, 
                                                 &wszClientLicensorCert 
                                                 );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDoAcquireClientLicensorCertificate failed. "\
                     L"hr = 0x%x\n", hr );
            goto e_Exit;
        }
    }
    else
    {
        wprintf( L"\nA client licensor certificate is "\
                 L"already in the license store.\n" );

        wszClientLicensorCert = new WCHAR[ uiClientLicensorCertLength ];
        if ( NULL == wszClientLicensorCert )
        {
            wprintf( L"\nMemory allocation of wszClientLicensorCert "\
                     L"failed.\n" );
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        hr = DRMEnumerateLicense( hClient, 
                                  DRM_EL_SPECIFIED_CLIENTLICENSOR,
                                  0,
                                  &fShared,
                                  &uiClientLicensorCertLength,
                                  wszClientLicensorCert
                                  );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMEnumerateLicense(DRM_EL_SPECIFIED_CLIENTLICENSOR)"\
                L" failed. hr = 0x%x\n", hr );
            goto e_Exit;
        }
    }

    //
    // 9. Read the manifest file into a buffer
    //
    hr = ReadFileToWideString( wszManifestFileName, &wszManifest );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nReadFileToWideString failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 10. Get the security provider
    //
    hr = DRMGetSecurityProvider( 0,
                                 &uiSecurityProviderTypeLength,
                                 NULL,
                                 &uiSecurityProviderPathLength,
                                 NULL
                                 );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMGetSecurityProvider failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszSecurityProviderType = new WCHAR[ uiSecurityProviderTypeLength ];
    if ( NULL == wszSecurityProviderType )
    {
        wprintf( L"\nMemory allocation for wszSecurityProviderType "\
                 L"failed.\n" );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    wszSecurityProviderPath = new WCHAR[ uiSecurityProviderPathLength ];
    if ( NULL == wszSecurityProviderPath )
    {
        wprintf( L"\nMemory allocation for wszSecurityProviderPath "\
                 L"failed." );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    hr = DRMGetSecurityProvider( 0,
                                 &uiSecurityProviderTypeLength,
                                 wszSecurityProviderType,
                                 &uiSecurityProviderPathLength,
                                 wszSecurityProviderPath
                                 );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMGetSecurityProvider failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }
                           
    //
    // 11. Initialize an environment
    //
    hr = DRMInitEnvironment( DRMSECURITYPROVIDERTYPE_SOFTWARESECREP,
                             DRMSPECTYPE_FILENAME,
                             wszSecurityProviderPath,
                             wszManifest,
                             wszMachineCertificate,
                             &hEnv,
                             &hLib
                             );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMInitEnvironment failed. hr = 0x%x.\n", hr );
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
    // 12. Create an unsigned issuance license from scratch
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

    //
    // Create a GUID to use as a unique content ID
    // in the issuance license
    //
    hr = CoCreateGuid( &guid );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nCoCreateGuid failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wszGUID = new WCHAR[ GUID_STRING_LENGTH ];
    if ( NULL == wszGUID )
    {
        wprintf( L"\nFailed to allocate memory for wszGUID\n" );
        goto e_Exit;
    }

    uiGUIDLength = StringFromGUID2( guid, wszGUID, GUID_STRING_LENGTH );
    if ( 0 == uiGUIDLength )
    {
        wprintf( L"\nStringFromGUID2 failed.\n" );
        hr = E_FAIL;
        goto e_Exit;
    }

    //
    // 13. Set the metadata, application specific data, interval time, and
    //     usage policy in the unsigned issuance license
    //
    hr = DRMSetMetaData( hIssuanceLicense, 
                         wszGUID, 
                         L"MS-GUID", 
                         SKU_ID, 
                         SKU_ID_TYPE, 
                         CONTENT_TYPE, 
                         CONTENT_NAME 
                         );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMSetMetaData failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Set the application specific data
    //
    hr = DRMSetApplicationSpecificData( hIssuanceLicense,
                                        false,
                                        APP_SPECIFIC_DATA_NAME,
                                        APP_SPECIFIC_DATA_VALUE
                                        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMSetApplicationSpecificData failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Set the interval time
    //
    hr = DRMSetIntervalTime( hIssuanceLicense, INTERVAL_TIME_DAYS );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMSetIntervalTime failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // Set the usage policy
    //
    hr = DRMSetUsagePolicy( hIssuanceLicense, 
                            DRM_USAGEPOLICY_TYPE_BYNAME,
                            false, 
                            true, 
                            USAGE_POLICY_APPLICATION_NAME,
                            USAGE_POLICY_MIN_VERSION, 
                            USAGE_POLICY_MAX_VERSION,
                            NULL, 
                            NULL, 
                            NULL, 
                            NULL 
                            );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMSetUsagePolicy failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 14. Create a user and right and add that to 
    //     the unsigned issuance license
    //
    hr = DRMCreateUser( wszUserId, NULL, L"Windows", &hUser );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateUser failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    hr = DRMCreateRight( RIGHT_NAME, 
                         &stTimeFrom, 
                         &stTimeUntil, 
                         0, 
                         NULL, 
                         NULL, 
                         &hRight 
                         );
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
    // 15. Sign the issuance license offline
    //
    hr = DRMGetSignedIssuanceLicense( hEnv, 
                                      hIssuanceLicense, 
                                      DRM_SIGN_OFFLINE | DRM_AUTO_GENERATE_KEY, 
                                      NULL,
                                      0, 
                                      L"AES", 
                                      wszClientLicensorCert, 
                                      &StatusCallback, 
                                      wszLicensingSvr,
                                      ( VOID* )&context
                                      );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMGetSignedIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 16. Wait for the callback to return
    //
    dwWaitResult = WaitForSingleObject( context.hEvent, DW_WAIT_TIME_SECONDS );
    if ( WAIT_TIMEOUT == dwWaitResult )
    {
        wprintf( L"\nWaitForSingleObject timed out.\n" );
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

    wprintf( L"\nThe signed issuance license is: \n\n%s\n", context.wszData );

    wprintf( L"\nDRMGetSignedIssuanceLicense succeeded.\n" );

e_Exit:
    //
    // 17. Clean up and free memory
    //
    if ( NULL != hClient )
    {
        hr = DRMCloseSession( hClient );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMCloseSession failed while closing "\
                L"hClient.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != context.wszData )
    {
        delete [] context.wszData;
    }
    if ( NULL != context.hEvent )
    {
        CloseHandle( context.hEvent );
    }
    if ( NULL != hIssuanceLicense )
    {
        hr = DRMClosePubHandle( hIssuanceLicense );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMClosePubHandle failed while closing "\
                L"hIssuanceLicense.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != hUser )
    {
        hr = DRMClosePubHandle( hUser );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMClosePubHandle failed while closing "\
                L"hUser.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != hRight )
    {
        hr = DRMClosePubHandle( hRight );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMClosePubHandle failed while closing "\
                L"hRight.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != hLib )
    {
        hr = DRMCloseHandle( hLib );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMCloseHandle failed while closing "\
                L"hLib.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != hEnv )
    {
        hr = DRMCloseEnvironmentHandle( hEnv );
        if ( FAILED( hr ) )
        {
            wprintf( L"\nDRMCloseEnvironmentHandle failed while closing "\
                L"hEnv.  hr = 0x%x\n", hr );
        }
    }
    if ( NULL != wszGUID )
    {
        delete [] wszGUID;
    }
    if ( NULL != wszUserId )
    {
        delete [] wszUserId;
    }
    if ( NULL != wszActivationSvr )
    {
        delete [] wszActivationSvr;
    }
    if ( NULL != wszLicensingSvr )
    {
        delete [] wszLicensingSvr;
    }
    if ( NULL != wszManifestFileName )
    {
        delete [] wszManifestFileName;
    }
    if ( NULL != wszManifest )
    {
        delete [] wszManifest;
    }
    if ( NULL != wszClientLicensorCert )
    {
        delete [] wszClientLicensorCert;
    }
    if ( NULL != wszSecurityProviderType )
    {
        delete [] wszSecurityProviderType;
    }
    if ( NULL != wszSecurityProviderPath )
    {
        delete [] wszSecurityProviderPath;
    }
    if ( NULL != wszMachineCertificate )
    {
        delete [] wszMachineCertificate;
    }
    if ( FAILED( hr ) )
    {
        wprintf( L"\nOfflinePublishing failed. hr = 0x%x\n", hr );
    }
    return hr;
}
