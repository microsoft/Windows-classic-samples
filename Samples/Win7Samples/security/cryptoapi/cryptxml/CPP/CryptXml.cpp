// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"

/*****************************************************************************
  HrLoadFile

    Load file into allocated (*ppbData). 
    The caller must free the memory by LocalFree().
*****************************************************************************/
HRESULT
HrLoadFile(
    LPCWSTR     wszFileName,
    BYTE        **ppbData,
    DWORD       *pcbData
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       cbRead = 0;
    HRESULT     hr = S_OK;

    *ppbData = NULL;
    *pcbData = 0;

    hFile = CreateFileW( 
						wszFileName, 
                        GENERIC_READ,
                        0,
                        NULL, 
                        OPEN_EXISTING, 
                        0, 
                        NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    *pcbData = GetFileSize( hFile, NULL );
    if( *pcbData == 0 ) 
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    *ppbData = (PBYTE)LocalAlloc( LPTR, *pcbData );
    if( NULL == *ppbData )
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        
        goto CleanUp;
    }

    if( !ReadFile( hFile, *ppbData, *pcbData, &cbRead, NULL ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

CleanUp:

    if (hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(hFile);
    }

    if( FAILED(hr) )
    {
        if( NULL != *ppbData )
        {
            LocalFree( *ppbData );
        }

        *ppbData = NULL;
        *pcbData = 0;
    }

    return hr;
}

#define WSZ_CMD_VERIFY  L"VERIFY"
#define WSZ_CMD_SIGN    L"SIGN"

/*****************************************************************************
 Usage

*****************************************************************************/
void Usage()
{
    wprintf( L"CryptXml.exe [Options] {COMMAND}\r\n" );
    wprintf( L"    Options:\r\n" );
    wprintf( L"     -h {CNGAlgName}     - SIGN: Hash name, default - SHA1\r\n" );
    wprintf( L"     -cm {URI}           - SIGN: Canonicalization method, default - Exclusive C14N.\r\n" );
    wprintf( L"     -kid {ID}           - SIGN: <KeyInfo> Id attribute.\r\n" );
    wprintf( L"     -kv                 - SIGN: Create <KeyValue> from the key handle.\r\n" );
    wprintf( L"     -n {SubjectName}    - SIGN: Subject Name of certificate in MY store used to sign.\r\n" );
    wprintf( L"     -sid {ID}           - SIGN: <Signature> Id attribute.\r\n" );
    wprintf( L"\r\n    COMMAND:\r\n" );
    wprintf( L"        %s {FileIn}\r\n", WSZ_CMD_VERIFY );
    wprintf( L"          | Parse and verify signature.\r\n" );
    wprintf( L"        %s {FileOut} [FileIn XPath] {#RefId [file]} .... {#RefId [file]} \r\n", WSZ_CMD_SIGN );
    wprintf( L"          | Create XML Signature.\r\n" );
    wprintf( L"          | Both FileIn and XPath must be specified for Enveloped or Enveloping signature.\r\n" );
    wprintf( L"          | If [file] is not specified, then #RefId is internal reference in FileIn.\r\n" );
}

/*****************************************************************************
 wmain

*****************************************************************************/
DWORD
__cdecl
wmain(
    int     argc,
    LPWSTR  argv[]
    )
{
    HRESULT         hr = S_FALSE;
    int             i=0;
    LPCWSTR         wszFile = NULL;

    SIGN_PARA       Para = {0};

    Para.wszCanonicalizationMethod = wszURI_CANONICALIZATION_EXSLUSIVE_C14N;
    Para.wszHashAlgName = BCRYPT_SHA1_ALGORITHM;

    //
    // Options
    //

    for( i=1; i<argc; i++ )
    {
        if ( 0 == lstrcmpW (argv[i], L"/?") ||
             0 == lstrcmpW (argv[i], L"-?") ) 
        {
            Usage();
            goto CleanUp;
        }

        if( *argv[i] != L'-' )
            break;

        if ( 0 == lstrcmpW (argv[i], L"-kv") )
        {
            Para.fKV = TRUE;
        }
        else
        if ( 0 == lstrcmpW (argv[i], L"-cm") )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszCanonicalizationMethod = argv[++i];
        }
        else
        if ( 0 == lstrcmpW (argv[i], L"-h") )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszHashAlgName = argv[++i];
        }
        else
        if ( 0 == lstrcmpW (argv[i], L"-n") )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszSubject = argv[++i];
        }
        else
        if ( 0 == lstrcmpW (argv[i], L"-kid") )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszKeyInfoId = argv[++i];
        }
        else
        if ( 0 == lstrcmpW (argv[i], L"-sid") )
        {
            if( i+1 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszSignatureId = argv[++i];
        }
    }

    //
    // Commands
    //

    if( i >= argc )
    {
        goto InvalidCommandLine;
    }

    if( 0 == lstrcmpW( argv[i], WSZ_CMD_VERIFY ))
    {
        i++;
        if( i >= argc )
        {
            goto InvalidCommandLine;
        }

        wszFile = argv[i];

        hr = HrVerify( 
                                        wszFile 
                                        );

        if( FAILED(hr) )
        {
            goto CleanUp;
        }
    }
    else
    if( 0 == lstrcmpW( argv[i], WSZ_CMD_SIGN ))
    {
        i++;
        if( i+1 >= argc )
        {
            goto InvalidCommandLine;
        }

        wszFile = argv[i++];

        if( L'#' != *argv[i] )
        {
            if( i+2 >= argc )
            {
                goto InvalidCommandLine;
            }

            Para.wszFileIn  = argv[i++];
            Para.wszSignatureLocation = argv[i++];
        }
        
        if( i >= argc )
        {
            goto InvalidCommandLine;
        }

        // The rest of the command line must be {#Reference| [File] }

        hr = HrSign( 
                                        wszFile, 
                                        &Para, 
                                        (ULONG)(argc-i),
                                        &argv[i]
                                        );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }
    }
    else
    {
        goto InvalidCommandLine;
    }

    //
    // End
    //

    hr = S_OK;
    goto CleanUp;

InvalidCommandLine:

    wprintf( L"ERROR: Invalid command line.\r\n" );

CleanUp:

    if( FAILED(hr) )
    {
        wprintf( L"ERROR: 0x%08x\r\n", hr );
    }

    return 0;
}