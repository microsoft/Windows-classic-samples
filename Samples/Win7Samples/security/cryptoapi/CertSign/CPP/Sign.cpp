// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Acquire a private key associated with a certificate and use it for signing

This example shows how to acquire private key associated with a certificate,
determine its type (CAPI or CNG) and used it to signed hashed message. 
In addition it demonstrates creating hash using CAPI or CNG APIs
Please note: even though this sample shows CNG hash signed by CNG key and 
CAPI hash signed by CAPI key, it is possible to use CNG key to sign CAPI hash

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

//----------------------------------------------------------------------------------------------------------------
// 
//  Function:   HrCreateCNGHash()
//
// The caller must call LocalFree to release (*ppbHash)
//------------------------------------------------------------------------------------------------------------------
HRESULT
HrCreateCNGHash ( 
    LPCWSTR     wszHashAlgName,                    
    const BYTE  *pbData,
    ULONG       cbData,
    BYTE        **ppbHash,
    ULONG       *pcbHash
    )
{
    HRESULT hr = S_OK;

    //handle to default crypto privider on your machine
    BCRYPT_ALG_HANDLE hAlgorithm = NULL;

    //handle to hash object
    BCRYPT_HASH_HANDLE hHash = NULL;

    //hash object
    BYTE *pbHashObject = NULL;
    ULONG cbHashObject = 0;

    ULONG cbResult = 0; //not used in our case

    //initialize OUT parameters
    *ppbHash = NULL;
    *pcbHash = 0;

    //get a handle to a cryptographic provider
    hr = BCryptOpenAlgorithmProvider(
                                            &hAlgorithm, 
                                            wszHashAlgName,             //hash algorthm 
                                            NULL,                       //default MS provider; can be replaced with custom installed provider
                                            0
                                            );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    //first get size of HASH buffer
    hr =  BCryptGetProperty(
                                            hAlgorithm, 
                                            BCRYPT_OBJECT_LENGTH,   //name of the propery we retrieve
                                            (PUCHAR) &cbHashObject, //hash object
                                            sizeof(cbHashObject),   //size of hash object
                                            &cbResult,              //not used in our case
                                            0);                     //dwFlags
    if( FAILED(hr) )
    {
        goto CleanUp;
    }
    
    //now allocate memory for hash object
    pbHashObject = (BYTE*)LocalAlloc( LPTR, cbHashObject );
    if( NULL == pbHashObject )
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        
        goto CleanUp;
    }
    
    //create hash
    hr = BCryptCreateHash(
                                        hAlgorithm,             //handle to an algorithm provider
                                        &hHash,
                                        pbHashObject,
                                        cbHashObject,
                                        NULL,                   //pbSecret
                                        0,                      //cbSecret
                                        0);                     //dwFlags
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    //hash data
    hr = BCryptHashData(
                                        hHash, 
                                        (PUCHAR)pbData,         //data to be hashed
                                        cbData,                 //size of data to be hashed in BYTES
                                        0);                     //dwFlags
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    //now get size of final HASH buffer
    hr =  BCryptGetProperty(
                                        hAlgorithm, 
                                        BCRYPT_HASH_LENGTH, //name of the propery we retrieve
                                        (PUCHAR)pcbHash, 
                                        sizeof(*pcbHash), 
                                        &cbResult, //not used in our case
                                        0); //dwFlags
    if( FAILED(hr) )
    {
        goto CleanUp;
    }
    
    //now allocate memory for hash object
    *ppbHash = (BYTE*)LocalAlloc( LPTR, *pcbHash );
    if (NULL == *ppbHash)
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        goto CleanUp;
    }

    //compute the hash and receive it in the buffer
    hr = BCryptFinishHash(            hHash, 
                                        (PUCHAR) *ppbHash, 
                                        *pcbHash, 
                                        0); //dwFlags
    if( FAILED(hr) )
    {
        goto CleanUp;
    }
        
    hr = S_OK;

CleanUp:

    if( NULL != hHash )
    {
        BCryptDestroyHash(&hHash);
    }

    if( NULL != pbHashObject )
    {
        LocalFree(pbHashObject);
    }

    if( FAILED( hr ))
    {
        if( NULL != *ppbHash )
        {
            LocalFree( *ppbHash );
        }
        *ppbHash = NULL;
        *pcbHash = 0;
    }

    return hr;
}

//----------------------------------------------------------------------------------------------------------------
// 
//  Function:   HrSignCNGHash()
//
// The caller must call LocalFree to release (*ppbSignature)
//------------------------------------------------------------------------------------------------------------------
HRESULT
HrSignCNGHash(    
    NCRYPT_KEY_HANDLE   hKey,
    void                *pPaddingInfo,
    DWORD               dwFlag,
    const BYTE          *pbHash,
    ULONG               cbHash,
	BYTE                **ppbSignature,
    ULONG               *pcbSignature
    )
{
    HRESULT hr = S_OK;

    //initialize OUT parameters
    *ppbSignature = NULL;
    *pcbSignature = 0;

    //get a size of signature
    hr = NCryptSignHash(
                                            hKey,
                                            pPaddingInfo,
                                            (PBYTE)pbHash,
                                            cbHash,
                                            NULL,           //pbSignature
                                            0,              //The size, in bytes, of the pbSignature buffer
                                            pcbSignature,
                                            dwFlag);        //dwFlags

    if( FAILED(hr) )
    {
        goto CleanUp;
    }
        
    // allocate buffer for signature
    *ppbSignature = (BYTE*)LocalAlloc( LPTR, *pcbSignature );
    if (NULL == *ppbSignature)
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        
        goto CleanUp;
    }
    
    hr = NCryptSignHash(
                                            hKey,
                                            pPaddingInfo,
                                            (PBYTE)pbHash,
                                            cbHash,
                                            *ppbSignature,
                                            *pcbSignature, 
                                            pcbSignature,
                                            dwFlag); //dwFlags

    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    hr = S_OK;

CleanUp:

    if( FAILED(hr) )
    {
        if( NULL != *ppbSignature )
        {
            LocalFree( *ppbSignature );
        }
        *ppbSignature = NULL;
        *pcbSignature = 0;
    }

    return hr;
}

//----------------------------------------------------------------------------------------------------------------
// 
//  Function:   HrVerifySignature()
//
//------------------------------------------------------------------------------------------------------------------
HRESULT
HrVerifySignature(
    PCCERT_CONTEXT  pCertContext,
	LPCWSTR         wszHashAlgName,
    const BYTE      *pbData,
    ULONG           cbData,
    const BYTE      *pbSignature,
    DWORD           cbSignature
    )
{
    HRESULT hr = S_OK;

	BCRYPT_PKCS1_PADDING_INFO *pPKCS1PaddingInfo = NULL;
	DWORD	dwCngFlags = 0;

    //public key of a cert used to sign data
    BCRYPT_KEY_HANDLE   hKey = NULL;
    
    BYTE *pbHash = NULL;
    DWORD cbHash = 0;

    BCRYPT_PKCS1_PADDING_INFO PKCS1PaddingInfo = {0};
	PCCRYPT_OID_INFO pOidInfo = NULL;

    //get certificate public key
    if( !CryptImportPublicKeyInfoEx2(
                                X509_ASN_ENCODING,
                                &pCertContext->pCertInfo->SubjectPublicKeyInfo,
                                CRYPT_OID_INFO_PUBKEY_SIGN_KEY_FLAG,
                                NULL,
                                &hKey
                                ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }
    
    hr = HrCreateCNGHash ( 
                                wszHashAlgName,                    
                                pbData,
                                cbData,
                                &pbHash,
                                &cbHash
                                );
    if( FAILED( hr ))
    {
        goto CleanUp;
    }

    // =======================================================================
    // Verify
    //

	// TODO:
	// The production code must specify valid padding.
	// SAMPLE:
    //   This padding valid for RSA non PSS only:
	//

	pOidInfo = CryptFindOIDInfo(
								CRYPT_OID_INFO_OID_KEY,
								pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
								CRYPT_PUBKEY_ALG_OID_GROUP_ID
								);
	if( NULL != pOidInfo &&
		0 == lstrcmpW( pOidInfo->pwszCNGAlgid, L"RSA" ))
	{
		PKCS1PaddingInfo.pszAlgId = wszHashAlgName;
		pPKCS1PaddingInfo = &PKCS1PaddingInfo;
		dwCngFlags = BCRYPT_PAD_PKCS1;
	}

    hr = BCryptVerifySignature(
                                hKey,
                                pPKCS1PaddingInfo,
                                (PUCHAR)pbHash,
                                cbHash,
                                (PUCHAR)pbSignature,
                                cbSignature,
                                dwCngFlags
                                );

    if( FAILED(hr))
    {
        goto CleanUp;
    }

    hr = S_OK;

CleanUp:

    if( NULL != pbHash )
    {
        LocalFree( pbHash );
    }

    if( NULL != hKey )
    {
        BCryptDestroyKey(hKey);
    }

    return hr;
}

//----------------------------------------------------------------------------------------------------------------
// 
//  Function:   HrSignCAPI()
//
//------------------------------------------------------------------------------------------------------------------
HRESULT
HrSignCAPI(
    HCRYPTPROV  hProvider,
    DWORD       dwKeySpec,
    LPCWSTR     wszHashAlgName,                    
    const BYTE  *pbData,
    ULONG       cbData,
	PBYTE       *ppbSignature,
    DWORD       *pcbSignature
    )
{
	DWORD i;
	BYTE b;
	PCCRYPT_OID_INFO pOidInfo = NULL;
    HRESULT hr = S_OK;

    HCRYPTHASH  hHash = NULL;

    //initialize OUT parameters
    *ppbSignature = NULL;
    *pcbSignature = 0;
    
    // Find ALGID for
    pOidInfo = CryptFindOIDInfo(
                                            CRYPT_OID_INFO_NAME_KEY,
                                            (void*)wszHashAlgName,
                                            CRYPT_HASH_ALG_OID_GROUP_ID
                                            );

    if( NULL == pOidInfo || 
        IS_SPECIAL_OID_INFO_ALGID( pOidInfo->Algid ))
    {
        hr = CRYPT_E_UNKNOWN_ALGO;
        goto CleanUp;
    }

    //create hash
    if (!CryptCreateHash(
                                            hProvider,          //handle to an algorithm provider
                                            pOidInfo->Algid,    //hash algorithm 
                                            0,                  //hKey
                                            0,                  //dwFlags
                                            &hHash ))           //hash object 
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }
        
    //hash data
    if (!CryptHashData(
                                            hHash,        //hash object
                                            pbData,     //data to be hashed
                                            cbData,     //size of data to be hashed in BYTES
                                            0 ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //get a size of signature
    if( !CryptSignHash(    
                                            hHash,
                                            dwKeySpec, 
                                            NULL,       //sDescription, not supported, must be NULL
                                            0,          //dwFlags
                                            NULL,       //pbSignature
                                            pcbSignature ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    //now allocate memory for signature object
    *ppbSignature = (BYTE*)LocalAlloc( LPTR, *pcbSignature );
    if (NULL == *ppbSignature)
    {
        hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
        goto CleanUp;
    }
    
    //now sign it
    if( !CryptSignHash(    
                                            hHash,
                                            dwKeySpec,
                                            NULL, //sDescription, not supported, must be NULL
                                            0, //dwFlags
                                            *ppbSignature,
                                            pcbSignature))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

	//
	// Reverse bytes to Big Endian
	//

	//
	// TODO: Check if the keys is DSA, then Reverse R&S separately
	// at the middle of the buffer.
	//

	// works for non DSA keys only
    if( *pcbSignature > 1 )
    {
	    for( i=0; i<(*pcbSignature)/2; i++ )
	    {
		    b = (*ppbSignature)[i];
		    (*ppbSignature)[i] = (*ppbSignature)[*pcbSignature-i-1];
		     (*ppbSignature)[*pcbSignature-i-1] = b;
	    }
    }

    hr = S_OK;

CleanUp:

    if( NULL != hHash )
    {
        CryptDestroyHash(hHash);
    }

    if( FAILED(hr) )
    {
        if( NULL != *ppbSignature )
        {
            LocalFree( *ppbSignature );
        }
        *ppbSignature = NULL;
        *pcbSignature = 0;
    }

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
    wprintf( L"%s [Options] {SIGN|VERIFY} InputFile SignatureFile\n", wsName );
    wprintf( L"\tOptions:\n" );
    wprintf( L"\t        -s {STORENAME}   : store name, (by default \"MY\")\n" );
    wprintf( L"\t        -n {SubjectName} : Certificate CN to search for, (by default \"Test\")\n" );
    wprintf( L"\t        -h {HashAlgName} : hash algorithm name, (by default \"SHA1\")\n" );
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
    HRESULT hr = S_OK;

    BOOL    fSign = TRUE;        

    // The message to be signed.
    BYTE    *pbPlainText = NULL;
    ULONG   cbPlainText = 0;

    //certificate to be used to sign data
    PCCERT_CONTEXT pCertContext = NULL; 

    LPCWSTR pwszInputFile = NULL;
    LPCWSTR pwszSignatureFile = NULL;

    LPCWSTR pwszStoreName = L"MY"; // by default, MY

    //subject name string of certificate to be used in signing
    //choose what cert do you want to use - CAPI or CNG
    LPCWSTR pwszCName = L"Test";

    //choose what hash algorithm to use, default SHA1
    LPCWSTR pwszHashAlgName = L"SHA1";

    //variable that receives the handle of either the CryptoAPI provider or the CNG key
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hCryptProvOrNCryptKey = NULL;

    //handle to CSP; is being used with CAPI keys
    HCRYPTPROV hCSP = NULL;

    //handle to CNG private key; is being used with CNG keys only
    NCRYPT_KEY_HANDLE hCngKey = NULL;

	DWORD	dwCngFlags = 0;

    //TRUE if user needs to free handle to a private key
    BOOL fCallerFreeKey = TRUE;

    //hashed data and signature
    BYTE *pbHash = NULL;
    BYTE *pbSignature = NULL;

    //size of hashed data and of signature
    DWORD cbHash = 0;
    DWORD cbSignature = 0;

    //key spec; will be used to determine key type
    DWORD dwKeySpec = 0;

    BCRYPT_PKCS1_PADDING_INFO PKCS1PaddingInfo = {0};
	BCRYPT_PKCS1_PADDING_INFO *pPKCS1PaddingInfo = NULL;
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
            Usage( L"certsign.exe" );
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
        if ( lstrcmpW (argv[i], L"-n") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                goto CleanUp;
            }

            pwszCName = argv[++i];
        }
        else
        if ( lstrcmpW (argv[i], L"-h") == 0 )
        {
            if( i+1 >= argc )
            {
                hr = E_INVALIDARG;
                goto CleanUp;
            }

            pwszHashAlgName = argv[++i];
        }
    }

    if( i+2 >= argc )
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if( 0 == lstrcmpW (argv[i], L"SIGN"))
        fSign = TRUE;
    else
    if( 0 == lstrcmpW (argv[i], L"VERIFY"))
        fSign = FALSE;
    else
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pwszInputFile = argv[i+1];
    pwszSignatureFile = argv[i+2];

    //-------------------------------------------------------------------
    // Find the test certificate to be validated and obtain a pointer to it

    hr = HrFindCertificateBySubjectName(
                                            pwszStoreName,
                                            pwszCName,
                                            &pCertContext
                                            );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    //
    // Load file
    // 

    hr = HrLoadFile(
                                            pwszInputFile,
                                            &pbPlainText,
                                            &cbPlainText
                                            );
    if( FAILED(hr) )
    {
        wprintf( L"Unable to read file: %s\n", pwszInputFile );
        goto CleanUp;
    }

    if( fSign )
    {
        if( !CryptAcquireCertificatePrivateKey(
                                                pCertContext,
                                                CRYPT_ACQUIRE_ALLOW_NCRYPT_KEY_FLAG,	//
                                                NULL,                                   //Reserved for future use and must be NULL
                                                &hCryptProvOrNCryptKey,
                                                &dwKeySpec,
                                                &fCallerFreeKey)) //user should free key if TRUE is returned
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto CleanUp;
        }


        //
        // check whether we have CNG or CAPI key
        //

        switch( dwKeySpec ) 
        {
            case CERT_NCRYPT_KEY_SPEC: //CNG key
                {
                    hCngKey = (NCRYPT_KEY_HANDLE) hCryptProvOrNCryptKey;
                    
                    hr = HrCreateCNGHash(       pwszHashAlgName,
                                                pbPlainText,
                                                cbPlainText,
                                                &pbHash, 
                                                &cbHash 
                                                );
                    if( FAILED(hr) )
                    {
                        goto CleanUp;
                    }

					// TODO:
					// The production code must specify valid padding.
					// SAMPLE:
                    //   This padding valid for RSA non PSS only:
					//
				
					pOidInfo = CryptFindOIDInfo(
												CRYPT_OID_INFO_OID_KEY,
												pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
												CRYPT_PUBKEY_ALG_OID_GROUP_ID
												);
					if( NULL != pOidInfo &&
						0 == lstrcmpW( pOidInfo->pwszCNGAlgid, L"RSA" ))
					{
						PKCS1PaddingInfo.pszAlgId = pwszHashAlgName;
						pPKCS1PaddingInfo = &PKCS1PaddingInfo;
						dwCngFlags = BCRYPT_PAD_PKCS1;
					}

                    hr = HrSignCNGHash(         hCngKey,
                                                pPKCS1PaddingInfo,
                                                dwCngFlags,
                                                pbHash,
                                                cbHash,
                                                &pbSignature,
                                                &cbSignature
                                                );
                    if( FAILED(hr) )
                    {
                        goto CleanUp;
                    }

                    wprintf( L"Signed message using CNG key.\n");
                }
                break;

            case AT_SIGNATURE: //CAPI key        
            case AT_KEYEXCHANGE:
                {
                    //
                    // Legacy (pre-Vista) key
                    //

                    hCSP = (HCRYPTPROV)hCryptProvOrNCryptKey;

                    hr = HrSignCAPI(            hCSP,
                                                dwKeySpec,
                                                pwszHashAlgName,
                                                pbPlainText,
                                                cbPlainText,
                                                &pbSignature,
                                                &cbSignature
                                                );
                    if( FAILED(hr) )
                    {
                        goto CleanUp;
                    }

                    wprintf( L"Successfully signed message using legacy CSP key.\n");
                }
                break;

            default:
                wprintf( L"Unexpected dwKeySpec returned from CryptAcquireCertificatePrivateKey.\n");

        }

        hr = HrSaveFile(
                                    pwszSignatureFile,
                                    pbSignature,
                                    cbSignature
                                    );
        if( FAILED(hr) )
        {
            wprintf( L"Unable to save file: %s\n", pwszSignatureFile );
            goto CleanUp;
        }

		wprintf( L"Created signature file: %s\n", pwszSignatureFile );
    }
    else
    {
        hr = HrLoadFile(
                                    pwszSignatureFile,
                                    &pbSignature,
                                    &cbSignature
                                    );
        if( FAILED(hr) )
        {
            wprintf( L"Unable to read file: %s\n", pwszSignatureFile );
            goto CleanUp;
        }

        //
        // For Public Key operations use BCrypt
        //

        hr = HrVerifySignature(
                                    pCertContext,
                                    pwszHashAlgName,
                                    pbPlainText,
                                    cbPlainText,
                                    pbSignature,
                                    cbSignature
                                    );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        wprintf( L"Successfully verified signature.\n");
    }

    hr = S_OK;

CleanUp:

    //free CNG key or CAPI provider handle
    if( fCallerFreeKey )
    {
        switch (dwKeySpec) 
        {
            case CERT_NCRYPT_KEY_SPEC: //CNG key
                NCryptFreeObject( hCngKey );
                break;

            case AT_SIGNATURE: //CAPI key        
            case AT_KEYEXCHANGE: 
                CryptReleaseContext( hCSP, 0);
                break;
        }
    }
    
    if( NULL != pCertContext )
    {
        CertFreeCertificateContext(pCertContext);
    }

    if( NULL != pbHash )
    {
        LocalFree(pbHash);
    }

    if( NULL != pbPlainText )
    {
        LocalFree(pbPlainText);
    }

    if( NULL != pbSignature )
    {
        LocalFree(pbSignature);
    }

    if( FAILED( hr ))
    {
        ReportError( NULL, hr  );
    }

    return (DWORD)hr;
}
