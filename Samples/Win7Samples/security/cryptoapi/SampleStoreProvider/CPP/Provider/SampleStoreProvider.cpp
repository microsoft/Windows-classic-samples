// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This sample application receives a certificate file name as a paramater;
// The certificate is added to a store implemented in SampleStore.dll

#define CRYPT_OID_INFO_HAS_EXTRA_FIELDS

#include <windows.h>
#include <winerror.h>
#include <strsafe.h>
#include <wincrypt.h>
#include <stdio.h>

/*****************************************************************************
 ReportError

	Prints error information to the console
*****************************************************************************/
void 
ReportError( 
    LPCWSTR     wszMessage, 
    DWORD       dwErrCode 
    )
{
	LPWSTR pwszMsgBuf = NULL;

	if( NULL!=wszMessage && 0!=*wszMessage )
    {
        wprintf( L"%s\n", wszMessage );
    }

	FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,                                       // Location of message
                                                    //  definition ignored
        dwErrCode,                                  // Message identifier for
                                                    //  the requested message    
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Language identifier for
                                                    //  the requested message
        (LPWSTR) &pwszMsgBuf,                       // Buffer that receives
                                                    //  the formatted message
        0,                                          // Size of output buffer
                                                    //  not needed as allocate
                                                    //  buffer flag is set
        NULL                                        // Array of insert values
		);
	
	if( NULL != pwszMsgBuf )
	{
	    wprintf( L"Error: 0x%08x (%d) %s\n", dwErrCode, dwErrCode, pwszMsgBuf );
		LocalFree(pwszMsgBuf);
	}
	else
	{
	    wprintf( L"Error: 0x%08x (%d)\n", dwErrCode, dwErrCode );
	}
}

/*****************************************************************************
 Usage

*****************************************************************************/
void 
Usage( 
    LPCWSTR wsName 
    )
{
    wprintf( L"%s [Options] {CertPath}\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t        -p {PROVIDER}      : provider name (by default \"TestExt\")\n" );
    wprintf( L"\t        -s {STORENAME}     : store name (by default \"TestStoreName\")\n" );
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
    HRESULT                 hr = S_OK;

    HCERTSTORE              hStore = NULL;
    PCCERT_CONTEXT          pCert = NULL;
    PCCERT_CONTEXT          pStoreCert = NULL;

    char                    szStoreProvider[ 256 ] = { "TestExt" };

    // Store provider name
    LPWSTR pwszStoreProvider = NULL;
    // Store name
    LPWSTR pwszStoreName = L"TestStoreName";

    LPWSTR pwszCertPath = NULL;


    int i;

    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage( L"SampleStoreProvider.exe" );
            goto CleanUp;
        }

        if( *argv[i] != L'-' )
            break;

        if ( lstrcmpW (argv[i], L"-s") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            pwszStoreName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-p") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            pwszStoreProvider = argv[++i];
        }

    }

    if( i >= argc )
    {
        hr = E_INVALIDARG;
        
        goto CleanUp;
    }

    pwszCertPath = argv[i++];

    // Load the certificate from the file passed in as cmd line parameter;    
    if (!CryptQueryObject(
                                CERT_QUERY_OBJECT_FILE,         //dwObjectType 
                                pwszCertPath,                   //pvObject
                                CERT_QUERY_CONTENT_FLAG_CERT,   //dwExpectedContentTypeFlags
                                CERT_QUERY_FORMAT_FLAG_ALL,     //dwExpectedFormatTypeFlags 
                                0,                              //dwFlags
                                0,                              //pdwMsgAndCertEncodingType 
                                0,                              //pdwContentType 
                                0,                              //pdwFormatType 
                                0,                              //phCertStore
                                0,                              //phMsg 
                                (const void**)&pCert            //ppvContext
                                ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    // Open the store using the sample provider name
    if( NULL != pwszStoreProvider )
    {
        if( 1 < WideCharToMultiByte(
                                CP_ACP, 
                                WC_ERR_INVALID_CHARS, 
                                pwszStoreProvider,
                                -1, 
                                szStoreProvider, 
                                sizeof(szStoreProvider),
                                NULL,    
                                NULL
                                ))
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            
            goto CleanUp;
        }
    }

    hStore = CertOpenStore(
                                szStoreProvider,        // lpszStoreProvider
                                X509_ASN_ENCODING,      // dwMsgAndCertEncodingType
                                NULL,                   // hCryptProv
                                0,                      // dwFlags (will create it if does not exists)
                                (void*)pwszStoreName);  // pvPara
    if (hStore == NULL)
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    // Add a certificate to store
    if(!CertAddCertificateContextToStore(
                                hStore,                             // hCertStore
                                pCert,                              // pCertContext
                                CERT_STORE_ADD_REPLACE_EXISTING,    // dwAddDisposition
                                NULL                                // ppStoreContext
                                ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }


    pStoreCert = CertFindCertificateInStore(
                                hStore,                             // hCertStore
                                X509_ASN_ENCODING,                  // dwCertEncodingType
                                0,                                  // dwFindFlags
                                CERT_FIND_EXISTING,                 // dwFindType        
                                pCert,                              // pvFindPara
                                NULL                                // pPrevCertContext
                                );
    if (pStoreCert == NULL)
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    hr = S_OK;

CleanUp:

    if( NULL != pCert )
        CertFreeCertificateContext(pCert);
    
    if( NULL != pStoreCert )
        CertFreeCertificateContext(pStoreCert);

    if( NULL != hStore )
        CertCloseStore(hStore, 0);

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
}
