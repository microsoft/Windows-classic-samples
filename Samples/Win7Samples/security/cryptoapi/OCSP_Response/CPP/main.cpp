// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************

Title: Retrieve and embed an OCSP response

This sample shows how to staple an OCSP response as a property on a certificate and use it for validation.
The sample also shows how to retrieve the OCSP response from the revocation information in a chain.

******************************************/

#define CRYPT_OID_INFO_HAS_EXTRA_FIELDS
#define CERT_CHAIN_PARA_HAS_EXTRA_FIELDS

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

//----------------------------------------------------------------------------
// HrFindCertificateBySubjectName
//
//----------------------------------------------------------------------------
HRESULT
HrFindCertificateBySubjectName(
    LPCWSTR			wszStore,
    LPCWSTR			wszSubject,
    PCCERT_CONTEXT	*ppcCert
    )
{
    HRESULT hr = S_OK;
    HCERTSTORE  hStoreHandle = NULL;  // The system store handle.

    *ppcCert = NULL;

    //-------------------------------------------------------------------
    // Open the certificate store to be searched.

    hStoreHandle = CertOpenStore(
                           CERT_STORE_PROV_SYSTEM,          // the store provider type
                           0,                               // the encoding type is not needed
                           NULL,                            // use the default HCRYPTPROV
                           CERT_SYSTEM_STORE_CURRENT_USER,  // set the store location in a 
                                                            //  registry location
                           wszStore
                           );                               // the store name 

    if( NULL == hStoreHandle )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    //-------------------------------------------------------------------
    // Get a certificate that has the specified Subject Name

    *ppcCert = CertFindCertificateInStore(
                           hStoreHandle,
                           X509_ASN_ENCODING ,        // Use X509_ASN_ENCODING
                           0,                         // No dwFlags needed
                           CERT_FIND_SUBJECT_STR,     // Find a certificate with a
                                                      //  subject that matches the 
                                                      //  string in the next parameter
                           wszSubject,                // The Unicode string to be found
                                                      //  in a certificate's subject
                           NULL);                     // NULL for the first call to the
                                                      //  function; In all subsequent
                                                      //  calls, it is the last pointer
                                                      //  returned by the function
    if( NULL == *ppcCert )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

CleanUp:

    if(NULL != hStoreHandle)
    {
        CertCloseStore( hStoreHandle, 0);
    }   

    return hr;
}

/*****************************************************************************
  HrLoadFile

    Load file into allocated (*ppbData). 
    The caller must free the memory by LocalFree().
*****************************************************************************/
HRESULT
HrLoadFile(
	LPCWSTR  wszFileName,
    PBYTE   *ppbData,
    DWORD   *pcbData
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       cbRead = 0;
    HRESULT     hr = S_OK;

    *ppbData = NULL;
    *pcbData = 0;

    hFile = CreateFileW( wszFileName, 
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

/*****************************************************************************
  HrSaveFile

*****************************************************************************/
HRESULT
HrSaveFile(
    LPCWSTR             wszFileName,
	PBYTE               pbData,
    DWORD               cbData
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HRESULT     hr = S_OK;
    DWORD       cbWritten = 0;

    hFile = CreateFileW( wszFileName, 
                        GENERIC_WRITE,
                        0,
                        NULL, 
                        CREATE_ALWAYS, 
                        0, 
                        NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    if( !WriteFile( hFile, pbData, cbData, &cbWritten, NULL ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

CleanUp:

    if (hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(hFile);
    }

    return hr;
}

/*****************************************************************************
 Usage

*****************************************************************************/
void 
Usage( 
    LPCWSTR wsName 
    )
{
    wprintf( L"%s [Options] {SubjectName} {OcspRespFile}\n", wsName );
    wprintf( L"  Options:\n" );
    wprintf( L"   -s STORENAME : store name, (by default MY)\n" );
    wprintf( L"   -staple      : staple certificate with {OcspRespFile} for verification,\n" );
    wprintf( L"                : otherwise OCSP response will be stored to {OcspRespFile}.\n" );
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
    HRESULT                     hr = S_OK;

    int                         i;

    LPCWSTR                     pwszStoreName = L"MY"; // by default, MY
    LPCWSTR                     pwszCName = NULL;

    PCCERT_CONTEXT              pCertContext = NULL; 

    LPCWSTR                     pwsOcspFilePath = NULL;
    BOOL                        fStaple = FALSE;
    DWORD                       cbOcspResponse;
    BYTE                        *pbOcspResponse = NULL;

    DWORD                       dwFlags = CERT_CHAIN_REVOCATION_CHECK_END_CERT;

    PCCERT_CHAIN_CONTEXT        pChain = NULL;
    CERT_CHAIN_PARA             ChainPara = {0};
    CERT_CHAIN_POLICY_PARA      ChainPolicy       = {0};
    CERT_CHAIN_POLICY_STATUS    PolicyStatus      = {0};

    PCRL_INFO                   pOSCPInfo = NULL;
    PCERT_EXTENSION             pOSCPExts = NULL;

    //we want to make sure that our sample uses OCSP response, which was stapled (set as a property) to certificate context
    //to prevent using cached OCSP response we will set ChainPara.pftCacheResync to current time
    //this will ensure that previously cached OCSP entries will not be used
    //in real scenarios, when stapling and retrieving are done on different machines, this is not nessessary

    FILETIME SystemTimeAsFileTime;
    GetSystemTimeAsFileTime (&SystemTimeAsFileTime);
    ChainPara.pftCacheResync = &SystemTimeAsFileTime;

    ChainPara.cbSize = sizeof(CERT_CHAIN_PARA);


    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage( L"OCSP_Response.exe" );
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
        if ( lstrcmpW (argv[i], L"-staple") == 0 )
        {
            fStaple = TRUE;
        }
    }

    if( i+1 >= argc )
    {
        hr = S_FALSE;
        
        Usage( L"OCSP_Response.exe" );
        goto CleanUp;
    }

    pwszCName = argv[i++];
    pwsOcspFilePath = argv[i];

    //find certificate in user store to be use to sign data
    hr = HrFindCertificateBySubjectName(
                                        pwszStoreName,
                                        pwszCName,
                                        &pCertContext
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    if( fStaple )
    {
        //
        // Load OCSP Response and staple it to the certificate context.
        //
        
        hr = HrLoadFile(
                                        pwsOcspFilePath,
                                        &pbOcspResponse,
                                        &cbOcspResponse
                                        );
        if( FAILED(hr) )
        {
            wprintf( L"Unable to read file: '%s'\n", pwsOcspFilePath );
            
            goto CleanUp;
        }

        //
        // CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY indicates to use cached OCSP response.
        //

        dwFlags |= CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY;

        //
        // staple OCSP response extension to certificate
        // 

        CRYPT_DATA_BLOB blob = {cbOcspResponse, pbOcspResponse};

        if( !CertSetCertificateContextProperty(
                    pCertContext,                           // Cert
                    CERT_OCSP_RESPONSE_PROP_ID,             //The property to be set
                    0,                                      //dwFlags
                    &blob 
                    ))                                      //OCSP Response extension blob
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }
        
        wprintf( L"The certificate stapled with OCSP response from the file.\n" );
    }

    //
    // Build chain and verify revocation on the end certificate
    //

    if(!CertGetCertificateChain(
                NULL,                                                                            // hEngine,
                pCertContext,                                                                    // Cert
                NULL,                                                                            // Time
                NULL,                                                                            // Additional store
                &ChainPara,                                                                      // Input parameters
                dwFlags,                                                                         // Flags
                NULL,                                                                            // Reserved
                &pChain))                                                                        // Out context
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    wprintf( L"The certificate chain built.\n" );

    //
    // Verify that the chain complies with Base policy
    //

    ChainPolicy.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
    ChainPolicy.dwFlags = 0;

    PolicyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

    ChainPolicy.pvExtraPolicyPara = NULL;
    if( !CertVerifyCertificateChainPolicy(
                                    CERT_CHAIN_POLICY_BASE,
                                    pChain,
                                    &ChainPolicy,
                                    &PolicyStatus))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    if( PolicyStatus.dwError != S_OK ) 
    {
        ReportError( L"Base Policy Chain Status Failure:", PolicyStatus.dwError  );
        hr = PolicyStatus.dwError;
        goto CleanUp;
    }

    if( !fStaple )
    {
        //
        // we want to make sure the revocation information was obtained from OCSP response
        //

        if( NULL == pChain->rgpChain[0]->rgpElement[0]->pRevocationInfo ||
            NULL == pChain->rgpChain[0]->rgpElement[0]->pRevocationInfo->pCrlInfo ||
            NULL == pChain->rgpChain[0]->rgpElement[0]->pRevocationInfo->pCrlInfo->pBaseCrlContext ||
            NULL == pChain->rgpChain[0]->rgpElement[0]->pRevocationInfo->pCrlInfo->pBaseCrlContext->pCrlInfo
            )
        {
            hr = CRYPT_E_NOT_FOUND;
            
            goto CleanUp;
        }

        pOSCPInfo = pChain->rgpChain[0]->rgpElement[0]->pRevocationInfo->pCrlInfo->pBaseCrlContext->pCrlInfo;

        //get OCSP Response extension from CRL_INFO
        pOSCPExts = CertFindExtension(
                        szOID_PKIX_OCSP_BASIC_SIGNED_RESPONSE,    //object identifier (OID) to use in the search
                        pOSCPInfo->cExtension,                    //Number of extensions in the rgExtensions array
                        pOSCPInfo->rgExtension);                //Array of CERT_EXTENSION structures

        if (NULL == pOSCPExts)
        {
            hr = CRYPT_E_NOT_FOUND;
            wprintf( L"OCSP response not found for the certificate.\n");
            goto CleanUp;
        }

        wprintf( L"The certificate chain contains OCSP response for the certificate.\n" );

        hr = HrSaveFile(
                                    pwsOcspFilePath,
                                    pOSCPExts->Value.pbData,
                                    pOSCPExts->Value.cbData
                                    );
        if( FAILED(hr) )
        {
            wprintf( L"Unable to save file: %s\n", pwsOcspFilePath );
            goto CleanUp;
        }

        wprintf( L"Successfully saved OCSP response.\n");
    }

    hr = S_OK;

CleanUp:

    if( NULL != pbOcspResponse )
    {
        LocalFree( pbOcspResponse );
    }

    if( NULL != pCertContext )
    {
        CertFreeCertificateContext(pCertContext);
    }

    if( NULL != pChain )
    {
        CertFreeCertificateChain(pChain);
    }

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
}
