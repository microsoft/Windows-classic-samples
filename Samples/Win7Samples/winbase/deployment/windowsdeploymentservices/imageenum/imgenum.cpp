/*++
Copyright (c) 2006 Microsoft Corporation

Module Name:

    imgenum.cpp

Abstract:

    Sample program to demonstrate usage of the WDS Client API.

--*/


#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "WdsClientApi.h"


//
// Helpful definitions.
//

#define CLEANUP_ON_HR_FAIL( hr, CleanupLabel )      \
    if ( FAILED( hr ) )                             \
    {                                               \
        goto CleanupLabel;                          \
    }


//
// Declaration of utility functions.
//

HRESULT
MyLoadStringW(
    __in HINSTANCE hInstance,
    __in UINT uID,
    __inout_ecount( cchBuffer ) PWSTR wszMsg,
    __in ULONG cchBuffer );


//
// Main application entry point.
//

int __cdecl
wmain(
    int argc,
    __in_ecount(argc) wchar_t *argv[ ],
    __in wchar_t *envp[ ] )
{
    HRESULT hr = S_OK,
            hrCleanup = S_OK;

    WDS_CLI_CRED Cred;

    PWSTR pwszArchitecture = NULL,
          pwszDescription = NULL,
          pwszName = NULL,
          pwszPath = NULL,
          pwszServer = NULL,
          pwszErrorMessage = NULL;

    DWORD dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN,
          dwIndex = 0,
          dwRetCode = 0;

    HANDLE hSession = NULL,
           hFind = NULL;

    WCHAR wszMsg[1024] = { 0 },
          wszArchitecture[20] = { 0 };


    if ( argc < 4 )
    {
        hr = MyLoadStringW( NULL, IDS_STRING_USAGE, wszMsg, ARRAYSIZE( wszMsg ) );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );
        wprintf( wszMsg, argv[0] );
        return 1;
    }

    //
    // Initialize the Credentials stucture with the passed in data.
    //
    
    ZeroMemory( &Cred, sizeof( Cred ) );
    Cred.pwszUserName = argv[1];
    Cred.pwszDomain = NULL;
    Cred.pwszPassword = argv[2];

    //
    // Get the server name.
    //

    pwszServer = argv[3];

    //
    // Create the session object.
    //

    hr = WdsCliCreateSession( pwszServer, NULL, &hSession );
    CLEANUP_ON_HR_FAIL( hr, Cleanup );

    //
    // Convert this to an authenticated session object.
    //

    hr = WdsCliAuthorizeSession( hSession, &Cred );
    CLEANUP_ON_HR_FAIL( hr, Cleanup );

    //
    // Start image enumeration.  This is the first communication with the server.
    //

    hr = WdsCliFindFirstImage( hSession, &hFind );
    CLEANUP_ON_HR_FAIL( hr, Cleanup );

    //
    // Print out a nice header.
    //
    hr = MyLoadStringW( NULL, IDS_STRING_HEADER, wszMsg, ARRAYSIZE( wszMsg ) );
    CLEANUP_ON_HR_FAIL( hr, Cleanup );

    wprintf( wszMsg );

    do
    {
        //
        // Initialize all variables on each iteration.
        //

        pwszName = NULL;
        pwszDescription = NULL;
        pwszPath = NULL;
        pwszArchitecture = NULL;
        dwIndex = 0;
        dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;

        //
        // Get the image attributes for the current image.
        //

        hr = WdsCliGetImageName( hFind, &pwszName );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        hr = WdsCliGetImageDescription( hFind, &pwszDescription );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        hr = WdsCliGetImagePath( hFind, &pwszPath );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        hr = WdsCliGetImageIndex( hFind, &dwIndex );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        hr = WdsCliGetImageArchitecture( hFind, &dwArchitecture );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        //
        // Convert the architecture value to string form.
        //

        switch ( dwArchitecture )
        {
        
        case PROCESSOR_ARCHITECTURE_INTEL:
            
            hr = MyLoadStringW( NULL, IDS_STRING_ARCH_X86, wszArchitecture, ARRAYSIZE( wszArchitecture ) );
            CLEANUP_ON_HR_FAIL( hr, Cleanup );

            pwszArchitecture = wszArchitecture;
            break;

        case PROCESSOR_ARCHITECTURE_AMD64:

            hr = MyLoadStringW( NULL, IDS_STRING_ARCH_X64, wszArchitecture, ARRAYSIZE( wszArchitecture ) );
            CLEANUP_ON_HR_FAIL( hr, Cleanup );

            pwszArchitecture = wszArchitecture;
            break;

        case PROCESSOR_ARCHITECTURE_IA64:
            hr = MyLoadStringW( NULL, IDS_STRING_ARCH_IA64, wszArchitecture, ARRAYSIZE( wszArchitecture ) );
            CLEANUP_ON_HR_FAIL( hr, Cleanup );

            pwszArchitecture = wszArchitecture;
            break;

        default:

            hr = MyLoadStringW( NULL, IDS_STRING_ARCH_UNKNOWN, wszArchitecture, ARRAYSIZE( wszArchitecture ) );
            CLEANUP_ON_HR_FAIL( hr, Cleanup );

            pwszArchitecture = wszArchitecture;
            break;

        }

        //
        // Print out the properties for each image.
        //

        hr = MyLoadStringW( NULL, IDS_STRING_ENTRY, wszMsg, ARRAYSIZE( wszMsg ) );
        CLEANUP_ON_HR_FAIL( hr, Cleanup );

        wprintf( wszMsg, pwszName, pwszDescription, pwszPath, dwIndex, pwszArchitecture );

    } while ( SUCCEEDED( hr = WdsCliFindNextImage( hFind ) ) );

    //
    // WdsCliFindNextImage returns ERROR_NO_MORE_FILES when it reaches the end of the enumeration.
    //

    if ( HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES ) == hr )
    {
        hr = S_OK;
    }

Cleanup:

    //
    // Close the search handle.
    //

    if ( hFind )
    {
        WdsCliClose( hFind );
    }

    //
    // Close the session.
    //

    if ( hSession )
    {
        WdsCliClose( hSession );
    }

    //
    // Return 0 on success and 1 on any failure.
    //

    if ( SUCCEEDED( hr ) )
    {
        hrCleanup = MyLoadStringW( NULL, IDS_STRING_SUCCESS, wszMsg, ARRAYSIZE( wszMsg ) );
        if ( SUCCEEDED( hrCleanup ) )
        {
            wprintf( wszMsg );
        }
        return 0;
    }
    else
    {
        hrCleanup = MyLoadStringW( NULL, IDS_STRING_FAILURE, wszMsg, ARRAYSIZE( wszMsg ) );
        
        if ( SUCCEEDED( hrCleanup ) )
        {
            wprintf( wszMsg, hr );

            //
            // Try to get the string representation of this error message.
            //

            FormatMessageW( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                hr,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                (PWSTR) &pwszErrorMessage,
                0,
                NULL );

            if ( pwszErrorMessage )
            {
                wprintf( L"%s", pwszErrorMessage );
                LocalFree( pwszErrorMessage );
            }
        }

        return 1;
    }
}


//
// Utility functions
//

HRESULT
MyLoadStringW(
    __in HINSTANCE hInstance,
    __in UINT uID,
    __inout_ecount( cchBuffer ) PWSTR wszMsg,
    __in ULONG cchBuffer )
{
    HRESULT hr = S_OK;
    int nChars = 0;

    // Load the string from the resource and then allocate
    // a buffer just big enough for it.
    //
    nChars = LoadStringW( hInstance, uID, wszMsg, cchBuffer );
    if ( 0 == nChars )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }

Cleanup:

    return hr;
}