// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// This file contains sample of CRYPT_XML_DATA_PROVIDER implementation
// to resolve extrenal references
//

#include "pch.h"

/****************************************************************************
 SAMPLE_FILE_DATA_PROVIDER_CLOSE

 This is a callback function for CRYPT_XML_DATA_PROVIDER,
 used to release the resources created by this data provider.

****************************************************************************/
static
HRESULT
CALLBACK 
SAMPLE_FILE_DATA_PROVIDER_CLOSE(
    void                *pvCallbackState
    )
{
    HRESULT hr = NO_ERROR;
    HANDLE hFile = (HANDLE)pvCallbackState;

    if( INVALID_HANDLE_VALUE != hFile )
    {
        if( !CloseHandle( hFile ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    return hr;
}

/****************************************************************************
 SAMPLE_FILE_DATA_PROVIDER_READ

 This is a callback function for CRYPT_XML_DATA_PROVIDER,
 used to read data from external reference (FILE).

****************************************************************************/
static
HRESULT
CALLBACK 
SAMPLE_FILE_DATA_PROVIDER_READ(
    void                *pvCallbackState,
	BYTE                *pbData,
    ULONG               cbData,
	ULONG               *pcbRead
    )
{
    HRESULT hr = NO_ERROR;
    HANDLE hFile = (HANDLE)pvCallbackState;

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = E_INVALIDARG;
    }
    else
    if( !ReadFile( 
                                    hFile,
                                    pbData,
                                    cbData,
                                    pcbRead,
                                    NULL 
                                    ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}


/****************************************************************************
 HrSampleResolveExternalXmlReference

 This function creates CRYPT_XML_DATA_PROVIDER for the provided URI.
 For the sample purposes, it supports local files only.

 For more information, see documentation on CryptXmlDigestReference.
****************************************************************************/
HRESULT 
WINAPI 
HrSampleResolveExternalXmlReference(
	LPCWSTR                 wszUri,
    CRYPT_XML_DATA_PROVIDER *pProviderOut
    )
{
    HRESULT                 hr = S_FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    static  WCHAR _wsHttp[] = L"http://";
    const ULONG cwHttp = 7;
    static  WCHAR _wsHttps[] = L"https://";
    const ULONG cwHttps = 8;

    static  WCHAR _wsFile[] = L"file://";
    const ULONG cwFile = 7;

    ULONG cw = 0;

    ZeroMemory( pProviderOut, sizeof( CRYPT_XML_DATA_PROVIDER ));

    if( NULL == wszUri || 0 == *wszUri )
    {
        goto CleanUp;
    }
    
    LPWSTR wsUri = (LPWSTR)wszUri;
    cw = lstrlenW( wsUri );

    //
    // Try http
    //

    if( ( cw > cwHttp && 
            CSTR_EQUAL == CompareStringW(
                                        LOCALE_NEUTRAL,
                                        NORM_IGNORECASE,
                                        wsUri,
                                        cwHttp,
                                        _wsHttp,
                                        cwHttp
                                        )) ||
        ( cw > cwHttps && 
            CSTR_EQUAL == CompareStringW(
                                        LOCALE_NEUTRAL,
                                        NORM_IGNORECASE,
                                        wsUri,
                                        cwHttps,
                                        _wsHttps,
                                        cwHttps
                                        )))
    {
        //
        // TODO: Retrive from HTTP
        //      Add here your implementation to retrieve URI from HTTP
        //

        hr = E_NOTIMPL;
        goto CleanUp;
    }
    else
    if( cw > cwFile && 
            CSTR_EQUAL == CompareStringW(
                                        LOCALE_NEUTRAL,
                                        NORM_IGNORECASE,
                                        wsUri,
                                        cwFile,
                                        _wsFile,
                                        cwFile
                                        ))
    {
        wsUri += cwFile;
    }
    else
    {
        //
        // Not supported schema
        //
        // hr = E_INVALIDARG;
        // goto CleanUp;

        // SAMPLE: here we accept URI without the schema, just a file name
    }

    //
    // Open the file
    //

    hFile = CreateFileW(
                                        wsUri,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );
    if( INVALID_HANDLE_VALUE != hFile )
    {
        pProviderOut->pvCallbackState = hFile;
        pProviderOut->cbBufferSize = 0;
        pProviderOut->pfnRead = SAMPLE_FILE_DATA_PROVIDER_READ;
        pProviderOut->pfnClose = SAMPLE_FILE_DATA_PROVIDER_CLOSE;

        hr = NO_ERROR;
        goto CleanUp;
    }

CleanUp:

    return hr;
}