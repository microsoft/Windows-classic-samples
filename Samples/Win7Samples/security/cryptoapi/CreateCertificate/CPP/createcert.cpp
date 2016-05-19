// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Creating a self-signed certificate

This example shows how to create a certificate with a private key 
associated with it.

To run this sample, first use CreateCertificate.exe /?

CreateCertificate.exe [Options] SubjectName
        Options:
                -c {Container}     : container name (by default "SAMPLE")
                -s {STORENAME}     : store name (by default "MY")
                -k {CNGAlgName}    : key algorithm name (by default "RSA")
                -h {CNGAlgName}    : hash algorithm name (by default "SHA1")

Specify the SubjectName with RDNs:
For example: CreateCertificate.exe "CN=TEST,OU=TESTOU"

****************************************************************/

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
 HrCreateKey

  This function uses the NCrypt API to open the storage provider
  and create the key
*****************************************************************************/
HRESULT
HrCreateKey(
    LPCWSTR             pwszProvName,
    LPCWSTR             wszContainerName,                                              
    LPCWSTR             pwszAlgid,
	DWORD				dwBits,
	NCRYPT_KEY_HANDLE   *phKey
    )
{
    NCRYPT_PROV_HANDLE  hProvider = NULL;
    NCRYPT_KEY_HANDLE   hKey = NULL;
    HRESULT             hr = S_OK;

    *phKey = NULL;

    hr = NCryptOpenStorageProvider(
                                        &hProvider,
                                        ( NULL != pwszProvName ) ? pwszProvName : MS_KEY_STORAGE_PROVIDER,
                                        0           // dwFlags
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    hr = NCryptOpenKey(
                                        hProvider,
                                        &hKey,
                                        wszContainerName,
                                        0,                    // dwLegacyKeySpec
                                        0                    // dwFlags
                                        );
    if( ERROR_SUCCESS == hr ) 
    {
        hr = NCryptDeleteKey(
                                      hKey,
                                      0                   // dwFlags
                                      );
        hKey = NULL;
        if( FAILED(hr) )
        {
            goto CleanUp;
        }
    } 

    hr = NCryptCreatePersistedKey(
                                        hProvider,
                                        &hKey,
                                        pwszAlgid,
                                        wszContainerName,
                                        0,                        // dwLegacyKeySpec
                                        0                        // dwFlags
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

	if( 0 != dwBits )
    {
        hr = NCryptSetProperty(
                                        hKey,
                                        NCRYPT_LENGTH_PROPERTY,
                                        (PBYTE) &dwBits,
                                        sizeof(dwBits),
                                        NCRYPT_PERSIST_FLAG
                                        );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }
    }

    hr = NCryptFinalizeKey(
                                        hKey,
                                        0                   // dwFlags
                                        );

    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    hr = S_OK;

CleanUp:

    if( FAILED( hr ) && NULL != hKey ) 
    {
        NCryptFreeObject( hKey );
        hKey = NULL;
    }

    if( NULL != hProvider ) 
    {
        NCryptFreeObject( hProvider );
    }

    *phKey = hKey;

    return hr;
}

/*****************************************************************************
 Usage

*****************************************************************************/
void 
Usage( 
    __in LPCWSTR wsName 
    )
{
    wprintf( L"%s [Options] SubjectName\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t        -c {Container}     : container name (by default \"SAMPLE\")\n" );
    wprintf( L"\t        -s {STORENAME}     : store name (by default \"MY\")\n" );
    wprintf( L"\t        -l {Bits}			: key size\n" );
    wprintf( L"\t        -k {CNGAlgName}    : key algorithm name (by default \"RSA\")\n" );
    wprintf( L"\t        -h {CNGAlgName}    : hash algorithm name (by default \"SHA1\")\n" );
}

/*****************************************************************************
 wmain

*****************************************************************************/
DWORD
__cdecl
wmain(
    __in                int     argc,
    __in_ecount(argc)   LPWSTR  argv[]
    )
{
    HRESULT             hr = S_OK;
    HCERTSTORE          hStoreHandle = NULL;
    LPCWSTR             wszStoreName = L"MY"; // by default, MY
    LPWSTR              wszContainerName= L"SAMPLE";
    LPCWSTR             wszSubject = NULL;
	DWORD				dwBits = 0;
    
    LPCWSTR             wszKeyAlgName = L"RSA"; //
    LPCWSTR             rgwszCNGAlgs[2]; //

    // Hash algorithm
    rgwszCNGAlgs[0] = L"SHA1";
    // Pub Key algorithm
    rgwszCNGAlgs[1] = L"RSA";

    NCRYPT_KEY_HANDLE   hCNGKey = NULL;
    PCCERT_CONTEXT        pCertContext         = NULL;
    CRYPT_KEY_PROV_INFO    KeyProvInfo = {0};
    CERT_NAME_BLOB      SubjectName             = { 0 };
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm = { 0 };
    PCCRYPT_OID_INFO pOidInfo = NULL;
    int i;

    //
    // options
    //

    for( i=1; i<argc; i++ )
    {
        if ( lstrcmpW (argv[i], L"/?") == 0 ||
             lstrcmpW (argv[i], L"-?") == 0 ) 
        {
            Usage( L"CreateCert.exe" );
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

            wszStoreName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-c") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            wszContainerName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-k") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            wszKeyAlgName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-h") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            rgwszCNGAlgs[0] = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-l") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                
                goto CleanUp;
            }

            dwBits = _wtol( argv[++i] );
        }
    }

    if( i >= argc )
    {
        hr = E_INVALIDARG;
        
        goto CleanUp;
    }

    wszSubject = argv[i];

    //
    // Find the Signature algorithm
    //

    memset( &SignatureAlgorithm, 0, sizeof( SignatureAlgorithm ));

    pOidInfo = CryptFindOIDInfo(
                                    CRYPT_OID_INFO_NAME_KEY,
                                    (void*)wszKeyAlgName,
                                    CRYPT_PUBKEY_ALG_OID_GROUP_ID
                                    );
    if( NULL == pOidInfo )
    {
        wprintf( L"FAILED: Unable to find Public Key algorithm: '%s'.\n", wszKeyAlgName );
        hr = CRYPT_E_UNKNOWN_ALGO;
        goto CleanUp;
    }

    if( NULL != pOidInfo->pwszCNGExtraAlgid && 0 != *pOidInfo->pwszCNGExtraAlgid )
    {
        rgwszCNGAlgs[1] = pOidInfo->pwszCNGExtraAlgid;
    }
    else
    {
        rgwszCNGAlgs[1] = pOidInfo->pwszCNGAlgid;
    }

    pOidInfo = CryptFindOIDInfo(
                                    CRYPT_OID_INFO_CNG_SIGN_KEY,
                                    rgwszCNGAlgs,
                                    CRYPT_SIGN_ALG_OID_GROUP_ID
                                    );
    if( NULL == pOidInfo )
    {
        wprintf( L"FAILED: Unable to find signature algorithm: '%s:%s'\n",
            rgwszCNGAlgs[0],
            rgwszCNGAlgs[1]
            );
        hr = CRYPT_E_UNKNOWN_ALGO;
        goto CleanUp;
    }

    SignatureAlgorithm.pszObjId = (LPSTR) pOidInfo->pszOID;


    //-------------------------------------------------------------------
    //  Open a system store, in this case, the My store.

    hStoreHandle  = CertOpenStore(
                                CERT_STORE_PROV_SYSTEM,          // The store provider type
                                0,                               // The encoding type is
                                                                 // not needed
                                NULL,                            // Use the default HCRYPTPROV
                                CERT_SYSTEM_STORE_CURRENT_USER,  // Set the store location in a
                                                                 // registry location
                                wszStoreName                     // The store name as a Unicode 
                                                                 // string
                                );
    if( NULL == hStoreHandle )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //+--------------------------------------------
    //    Create a certificate context and add it to the store.
    //
    //    The structure of a X.509 v3 digital certificate is as follows:
    //        Version 
    //        Serial Number 
    //        Algorithm ID 
    //        Issuer Name
    //        Validity 
    //            Not Before 
    //            Not After 
    //        Subject Name
    //        Subject Public Key Info 
    //            Public Key Algorithm 
    //            Subject Public Key 
    //        Issuer Unique Identifier (Optional) 
    //        Subject Unique Identifier (Optional) 
    //        Extensions (Optional) 
    //        Certificate Signature Algorithm 
    //        Certificate Signature 

    //---------------------------------------------

    if( !CertStrToName(
                        X509_ASN_ENCODING,                                      // Use X509_ASN_ENCODING
                        wszSubject,                                             // String to be encoded
                        CERT_X500_NAME_STR | CERT_NAME_STR_SEMICOLON_FLAG,      // Type of the string
                        NULL,                                                   // Reserved for future use
                        NULL,                                                   // Pointer to a buffer that receives the
                                                                                //  encoded structure; NULL to obtain the
                                                                                //  required size of the buffer
                                                                                //  for memory allocation purposes
                        &SubjectName.cbData,                                    // Size, in bytes, required for the buffer
                        NULL) )                                                 // Additional error information about 
																				//  an invalid input string
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
		wprintf( L"FAILED: CertStrToName('%s').\n", wszSubject);
        goto CleanUp;
    }

    SubjectName.pbData = (BYTE*)LocalAlloc( LPTR, SubjectName.cbData);
    if (NULL == SubjectName.pbData) 
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        goto CleanUp;
    }

    if (!CertStrToName(
                        X509_ASN_ENCODING,                                    // Use X509_ASN_ENCODING    
                        wszSubject,                                        // String to be encoded
                        CERT_X500_NAME_STR | CERT_NAME_STR_SEMICOLON_FLAG,  // Type of the string
                        NULL,                                                // Reserved for future use
                        SubjectName.pbData,                                    // Pointer to a buffer that receives the
                                                                            //  encoded structure
                        &SubjectName.cbData,                                // Size, in bytes, required for the buffer
                        NULL) )                                                // Additional error information about 
                                                                //  an invalid input string
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
		wprintf( L"FAILED: CertStrToName('%s').\n", wszSubject);
        goto CleanUp;
    }

    // Get a CERT_PUBLIC_KEY_INFO

    hr = HrCreateKey(
                        MS_KEY_STORAGE_PROVIDER,
                        wszContainerName,
                        wszKeyAlgName,
						dwBits,
                        &hCNGKey
                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    // Tie this certificate to the Private key by setting a property
    memset(&KeyProvInfo, 0, sizeof(KeyProvInfo));
    KeyProvInfo.pwszContainerName = wszContainerName;
    KeyProvInfo.pwszProvName = MS_KEY_STORAGE_PROVIDER;
    KeyProvInfo.dwProvType = 0;
    KeyProvInfo.dwKeySpec = 0;

    pCertContext = CertCreateSelfSignCertificate(
                                        NULL,                  // Use information from pKeyProvInfo
                                                               //  parameter to acquire the handle
                                                              //  to cryptographic provider
                                        &SubjectName,          // Pointer to a BLOB containing the
                                                              //  distinguished name (DN) for 
                                                              //  the certificate subject
                                        0,                      // No flags: use default behavior
                                        &KeyProvInfo,          // Key provider information 
                                        &SignatureAlgorithm,  // Use default signature algorithm
                                        NULL,                  // Start time : Use the system 
                                                              //  current time by default
                                        NULL,                  // End time : Use start time plus
                                                              //  one year by default
                                        NULL                  // Array of extensions
                                        );
    if (NULL == pCertContext) 
    { 
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    wprintf( L"Successfully created certificate.\n");

    // Add it to the Store
    if( !CertAddCertificateContextToStore(
                                        hStoreHandle,
                                        pCertContext,
                                        CERT_STORE_ADD_ALWAYS,
                                        NULL))    
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        
        goto CleanUp;
    }

    wprintf( L"Certificate added to the store.\n");

CleanUp: 

    //-------------------------------------------------------------------
    // Clean up memory.

    if( SubjectName.pbData)
    {
        LocalFree(SubjectName.pbData);
    }

    if( NULL != hCNGKey)
    {
        NCryptFreeObject(hCNGKey);
    }

    if( NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if( NULL != hStoreHandle) 
    {
        CertCloseStore( hStoreHandle, 0 );
    }

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
} // End of main
