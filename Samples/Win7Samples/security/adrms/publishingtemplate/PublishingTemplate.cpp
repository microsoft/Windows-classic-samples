/*=====================================================================
File:      PublishingTemplate.cpp

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.
=====================================================================*/

//
// This sample shows how to create a issuance license from an existing 
// template. See the comments at the beginning of wmain() for a more 
// detailed description.
//

#include <stdio.h>
#include <Wtypes.h>
#include <strsafe.h>

#include "msdrm.h"
#include "msdrmdefs.h"
#include "msdrmerror.h"


//
// Print the correct usage of this application
//
void PrintUsage()
{
    wprintf( L"Usage:\n" );
    wprintf( L"\n  PublishingTemplate -T Template\n" );
    wprintf( L"    -T: Template to use for the issuance license \n" );\
    wprintf( L"        example: myTemplate.xml\n" );
}

//
// Parse the values passed in through the command line
//
HRESULT 
ParseCommandLine( 
                 int argc, 
                 __in_ecount( argc )WCHAR **argv, 
                 __deref_out_opt PWCHAR *pwszTemplate
                 )
{
    HRESULT hr               = S_OK;
    size_t  uiTemplateLength = 0;

    //
    // Initialize parameters
    //
    *pwszTemplate = NULL;

    //
    // Check input parameters.
    //
    if ( ( 3 != argc ) )
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

        //
        // The issuance license template to use
        //
        if ( 'T' == toupper( argv[ i ][ 1 ] ) )
        {
            //
            // Retrieve the length of the template name
            //
            hr = StringCchLengthW( argv[ i + 1 ], 
                STRSAFE_MAX_CCH, 
                &uiTemplateLength 
                );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchLengthW failed.  hr = 0x%x\n", hr );
                break;
            }
            //
            // Allocate memory for the template name
            //
            *pwszTemplate = new WCHAR[ uiTemplateLength + 1 ];
            if ( NULL == *pwszTemplate ) 
            {
                wprintf( L"Failed to allocate memory for pwszPath\n" );
                hr = E_OUTOFMEMORY;
                break;
            }
            //
            // Copy the template name into the pwszTemplate buffer
            //
            hr = StringCchCopyW( ( wchar_t* )*pwszTemplate, 
                uiTemplateLength + 1 , 
                argv[ i + 1 ] 
            );
            if ( FAILED( hr ) )
            {
                wprintf( L"StringCchCopyW failed.  hr = 0x%x\n", hr );
                break;
            }
            i++;
        }
        else
        {
            hr = E_INVALIDARG;
            break;
        }
    }
    if ( NULL == *pwszTemplate )
    {
        wprintf( L"A template is required.\n" );
        hr = E_INVALIDARG;
    }
    return hr;
}

//
// Copy the contents of the file into a string buffer
//
HRESULT
ReadFileToWideString(
                     __in LPCWSTR szFileName,
                     __deref_out PWCHAR *pwszTemplateString
                     )
{
    HRESULT                    hr            = S_OK;
    HANDLE                     hFile         = INVALID_HANDLE_VALUE;
    DWORD                      dwBytesRead   = 0;
    BY_HANDLE_FILE_INFORMATION fileInfo;

    //
    // Validate the parameters
    //
    if ( ( NULL == szFileName ) ||
         ( FAILED( StringCchLengthW( ( LPCWSTR )szFileName, MAX_PATH, NULL ) ) ) ||
         ( NULL == pwszTemplateString ) )
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
        wprintf( L"CreateFileW failed. hFile == INVALID_HANDLE_VALUE\n" );
        hr = HRESULT_FROM_WIN32( GetLastError() ) ;
        goto e_Exit;
    }

    if ( 0 == GetFileInformationByHandle( hFile, &fileInfo ) )
    {
        wprintf( L"GetFileInformationByHandle failed.\n" );
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

    *pwszTemplateString = new WCHAR[ fileInfo.nFileSizeLow + sizeof( WCHAR ) ];
    if ( NULL == *pwszTemplateString )
    {
        wprintf( L"Memory allocation for pwszTemplateString failed." );
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    //
    // Put the contents of the file into pwszTemplateString
    //
    if ( 0 == ReadFile( hFile, 
                        *pwszTemplateString, 
                        fileInfo.nFileSizeLow, 
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
// This sample will perform the following actions:
//    1.  Put the contents of the template file into a PWSTR
//    2.  Create an unsigned issuance license from the template
//    3.  Clean up and free memory
//
int __cdecl 
wmain( 
      int argc, 
      __in_ecount( argc )WCHAR **argv 
      )
{
    HRESULT                   hr                 = E_FAIL;
    PWCHAR                    pwszTemplateString = NULL;
    PWCHAR                    wszTemplate        = NULL;
    DRMPUBHANDLE              hIssuanceLicense   = NULL;

    if ( FAILED ( ParseCommandLine( argc, argv, &wszTemplate ) ) )
    {
        PrintUsage();
        goto e_Exit;
    }

    wprintf( L"\nRunning sample PublishingTemplate...\n" );

    //
    //    1.  Put the contents of the template file into a PWSTR
    //
    hr = ReadFileToWideString( wszTemplate, &pwszTemplateString );
    if ( FAILED( hr ) )
    {
        wprintf( L"ReadFileToWideString failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    //
    // 2. Create an unsigned issuance license from the template
    //
    hr = DRMCreateIssuanceLicense( NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        pwszTemplateString,
        NULL,
        &hIssuanceLicense 
        );
    if ( FAILED( hr ) )
    {
        wprintf( L"\nDRMCreateIssuanceLicense failed. hr = 0x%x\n", hr );
        goto e_Exit;
    }

    wprintf( L"PublishingTemplate succeeded.\n" );

e_Exit:
    //
    // 3. Clean up and free memory
    //
    if ( NULL != pwszTemplateString )
    {
        delete [] pwszTemplateString;
    }
    if ( NULL != wszTemplate )
    {
        delete [] wszTemplate;
    }
    if ( NULL != hIssuanceLicense )
    {
        hr = DRMClosePubHandle( hIssuanceLicense );
        if ( FAILED( hr ) )
        {
            wprintf( L"DRMClosePubHandle failed while closing "\
                L"hIssuanceLicense.  hr = 0x%x\n", hr );
            hr = E_FAIL;
        }
    }
    if ( FAILED( hr ) )
    {
        wprintf( L"\nPublishingTemplate failed. hr = 0x%x\n", hr );
    }
    return hr;
}