// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"

/*****************************************************************************
 HrWriteXmlToFileCallback

  The callback for CryptXmlEncode, 
  used to write the XML Signature to file.

  CryptXmlEncode will call this function for each XML chunk available
  during encoding.

*****************************************************************************/
static
HRESULT 
CALLBACK
HrWriteXmlToFileCallback(
	void                *pvCallbackState, 
	const BYTE          *pbData, 
    ULONG               cbData
    )
{
    HRESULT hr = S_FALSE;
    HANDLE  hFile = (HANDLE)pvCallbackState;
    DWORD   dwNumberOfBytesWritten = 0;

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    else
    if( !WriteFile(
                    hFile,
                    pbData,
                    cbData,
                    &dwNumberOfBytesWritten,
                    NULL ))
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    hr = S_OK;

CleanUp:

    return hr;
}

/*****************************************************************************
 HrGetSignerKeyAndChain

  This function retrieves a signing certificate from the local user’s 
  certificate store, builds certificates chain and returns key handle 
  for the signing key.”

  NOTE:
  The phCryptProvOrNCryptKey is cached and must not be released by the caller.

*****************************************************************************/
static
HRESULT
HrGetSignerKeyAndChain(
    LPCWSTR                 wszSubject,
    PCCERT_CHAIN_CONTEXT    *ppChainContext,
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE* phCryptProvOrNCryptKey,    
    DWORD                   *pdwKeySpec
    )
{
    HRESULT         hr = S_FALSE;
    HCERTSTORE      hStore = NULL;
    PCCERT_CONTEXT  pCert = NULL;
    BOOL            fCallerFreeProvOrNCryptKey = FALSE;

    CERT_CHAIN_PARA  ChainPara         = {0};
    ChainPara.cbSize = sizeof(ChainPara);

    *ppChainContext = NULL;
    *phCryptProvOrNCryptKey = NULL;
    *pdwKeySpec = 0;

    //
    // Open the local user store to search for certificates
    //

    hStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            X509_ASN_ENCODING,
                            NULL,
                            CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                            L"MY"
                            );
    
    if( NULL == hStore )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

    if( NULL != wszSubject && 0 != *wszSubject )
    {
        //
        // Search by Name
        //

        while( NULL != ( pCert = CertFindCertificateInStore(
                            hStore,
                            X509_ASN_ENCODING,
                            0,
                            CERT_FIND_SUBJECT_STR,
                            wszSubject,
                            pCert
                            )))
        {
            if( CryptAcquireCertificatePrivateKey(
                            pCert,
                            CRYPT_ACQUIRE_CACHE_FLAG,
                            NULL,
                            phCryptProvOrNCryptKey,
                            pdwKeySpec,
                            &fCallerFreeProvOrNCryptKey
                            ))
            {
                break;
            }
        }
    }
    else
    {
        //
        // Get the first available certificate in the store
        //

        while( NULL != ( pCert = CertEnumCertificatesInStore(
                            hStore,
                            pCert
                            )))
        {
            if( CryptAcquireCertificatePrivateKey(
                            pCert,
                            CRYPT_ACQUIRE_CACHE_FLAG,
                            NULL,
                            phCryptProvOrNCryptKey,
                            pdwKeySpec,
                            &fCallerFreeProvOrNCryptKey
                            ))
            {
                break;
            }
        }
    }

    if( NULL == pCert )
    {
        hr = CRYPT_XML_E_SIGNER;
        goto CleanUp;
    }

    //
    // Build the certificate chain without revocation check.
    //

    if( !CertGetCertificateChain(
                                NULL,                   // use the default chain engine
                                pCert,                  // pointer to the end certificate
                                NULL,                   // use the default time
                                NULL,                   // search no additional stores
                                &ChainPara,            
                                0,                      // no revocation check
                                NULL,                   // currently reserved
                                ppChainContext ))       // return a pointer to the chain created
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto CleanUp;
    }

CleanUp:

    if( FAILED(hr) )
    {
        *phCryptProvOrNCryptKey = NULL;
        *pdwKeySpec = 0;
    }

    if( NULL != pCert )
    {
        CertFreeCertificateContext( pCert );
    }

    if( NULL != hStore )
    {
        CertCloseStore( hStore, 0 );
    }

    return hr;
}

/*****************************************************************************
 HrSign

    Creates XML signature
*****************************************************************************/
HRESULT
HrSign(
    LPCWSTR         wszFileOut,
    const SIGN_PARA *pPara,
    ULONG           argc,
    LPWSTR          argv[]
    )
{
    HCRYPTXML               hSig = NULL;
    HCRYPTXML               hRef = NULL;
    HRESULT                 hr = S_FALSE;
    ULONG                   i;

    const CRYPT_XML_ALGORITHM_INFO* pAlgInfo = NULL;

    PCCERT_CHAIN_CONTEXT    pChainContext = NULL;
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hCryptProvOrNCryptKey = NULL; // No release
    DWORD                   dwKeySpec = 0;
    PCCERT_CONTEXT          pCert = NULL;   // No release
    CRYPT_XML_STATUS    Status = {0};

    ULONG               cTransform = 0;
    CRYPT_XML_ALGORITHM *pTransform = NULL;

    const CRYPT_XML_REFERENCE *pRef = NULL;
    CRYPT_XML_DATA_PROVIDER DataProvider = {0};

    CRYPT_XML_PROPERTY  Properties[] = {
        {
            //
            // This property is required for Enveloped or Enveloping signatures
            //
            CRYPT_XML_PROPERTY_SIGNATURE_LOCATION,
            NULL,
            sizeof(LPCWSTR)
        },
    };
    ULONG   cProperties = 0;

    CRYPT_XML_BLOB   Encoded = { CRYPT_XML_CHARSET_AUTO, 0, NULL };

    CRYPT_XML_ALGORITHM xmlAlg_CanonicalizationMethod = {
                                sizeof( CRYPT_XML_ALGORITHM ),
                                (LPWSTR)pPara->wszCanonicalizationMethod,
                                CRYPT_XML_CHARSET_AUTO,
                                0,
                                NULL
                                };

    CRYPT_XML_ALGORITHM xmlAlg_SignatureMethod = {
                                sizeof( CRYPT_XML_ALGORITHM ),
                                NULL,
                                CRYPT_XML_CHARSET_AUTO,
                                0,
                                NULL
                                };

    CRYPT_XML_ALGORITHM xmlAlg_DigestMethod = {
                                sizeof( CRYPT_XML_ALGORITHM ),
                                NULL,
                                CRYPT_XML_CHARSET_AUTO,
                                0,
                                NULL
                                };

    CRYPT_XML_ALGORITHM xmlAlg_Enveloped = {
                                sizeof( CRYPT_XML_ALGORITHM ),
                                wszURI_XMLNS_TRANSFORM_ENVELOPED,
                                CRYPT_XML_CHARSET_AUTO,
                                0,
                                NULL
                                };
    
    HANDLE  hFile = INVALID_HANDLE_VALUE;

    //
    // Create the output file.
    // This handle will be used by HrWriteXmlToFileCallback and must be closed
    // at the exit.
    //

    hFile = CreateFile(
                                        wszFileOut,
                                        GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        wprintf( L"ERROR: Unable to create file: '%s'.\r\n", wszFileOut );

        goto CleanUp;
    }

    //
    // Find the signing certificate
    //

    hr = HrGetSignerKeyAndChain(
                                        pPara->wszSubject,
                                        &pChainContext,
                                        &hCryptProvOrNCryptKey,    
                                        &dwKeySpec
                                        );
    if( FAILED(hr) )
    {
        wprintf( L"ERROR: 0x%08x - Unable to get signing certificate.\r\n", hr );
        goto CleanUp;
    }

    //
    // Determine the Digest Method
    //

    {
        pAlgInfo = CryptXmlFindAlgorithmInfo(
                                        CRYPT_XML_ALGORITHM_INFO_FIND_BY_CNG_ALGID,
                                        pPara->wszHashAlgName,
                                        CRYPT_XML_GROUP_ID_HASH,
                                        0
                                        );
        if( NULL == pAlgInfo )
        {
            hr = CRYPT_XML_E_ALGORITHM;
            goto CleanUp;
        }

        xmlAlg_DigestMethod.wszAlgorithm = pAlgInfo->wszAlgorithmURI;
    }

    //
    // Determine the Signature Method
    //
    
    pCert = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;
    {
        PCCRYPT_OID_INFO pOIDInfo = NULL;
        LPCWSTR pwszCNGAlgid[2] = {0};

        //
        // First, find the Public Key algorithm name
        //

        pOIDInfo = CryptFindOIDInfo(
                                            CRYPT_OID_INFO_OID_KEY,
                                            pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
                                            CRYPT_PUBKEY_ALG_OID_GROUP_ID
                                            );

        if( NULL == pOIDInfo || NULL == pOIDInfo->pwszCNGAlgid )
        {
            hr = CRYPT_XML_E_ALGORITHM;
            goto CleanUp;
        }

        //
        // Second, find XML DigSig URI that corresponds to 
        // combined HASH and  Public Key algorithm names.
        //

        pwszCNGAlgid[0] = pPara->wszHashAlgName;
        pwszCNGAlgid[1] = pOIDInfo->pwszCNGAlgid;

        pAlgInfo = CryptXmlFindAlgorithmInfo(
                                            CRYPT_XML_ALGORITHM_INFO_FIND_BY_CNG_SIGN_ALGID,
                                            pwszCNGAlgid,
                                            CRYPT_XML_GROUP_ID_SIGN,
                                            0
                                            );
        if( NULL == pAlgInfo )
        {
            hr = CRYPT_XML_E_ALGORITHM;
            goto CleanUp;
        }
        xmlAlg_SignatureMethod.wszAlgorithm = pAlgInfo->wszAlgorithmURI;
    }

    //
    // Load input XML file. This must be provided for Enveloped or Enveloping signature
    //

    if( NULL != pPara->wszFileIn )
    {
        hr = HrLoadFile(
                                            pPara->wszFileIn,
                                            &Encoded.pbData,
                                            &Encoded.cbData
                                            );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }
    }

    //
    // Create the document context
    //

    if( NULL != pPara->wszSignatureLocation && 0 != *pPara->wszSignatureLocation )
    {
        // The <Signature> element will be added into this location
        Properties[0].pvValue = &pPara->wszSignatureLocation;
        cProperties++;
    }

    hr = CryptXmlOpenToEncode(
                                            NULL,                   // No custom transforms
                                            0,
                                            pPara->wszSignatureId,
                                            Properties,
                                            cProperties,
                                            (Encoded.cbData > 0 ) ? &Encoded : NULL,
                                            &hSig
                                            );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    //
    // Create references
    //

    for( i=0; i<argc; i++ )
    {
        DWORD       dwReferenceFlags = 0;
        LPCWSTR     wsRefId = NULL;
        LPCWSTR     wsUri = NULL;

        if( L'#' != *argv[i] )
        {
            wprintf( L"ERROR: Invalid reference: %s.\r\n", argv[i] );
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        wsUri = argv[i];
        
        if( 0 == wsUri[1] && 1==argc )
        {
            //
            // Special case for Enveloped
            // The URI must be ""
            //
            
            wsUri = L"";
            cTransform = 1;
            pTransform = &xmlAlg_Enveloped;
        }
        else
        if( i+1 < argc )
        {
            //
            // Check if external file is specified
            //

            if( L'#' != *argv[i+1] )
            {
                i++;
                wsRefId = wsUri+1;
                wsUri = argv[i];
            }

            cTransform = 0;
            pTransform = NULL;
        }

        hr = CryptXmlCreateReference(
                                        hSig,               // Parent
                                        dwReferenceFlags,   // Flags
                                        wsRefId,
                                        wsUri,
                                        NULL,
                                        &xmlAlg_DigestMethod,
                                        cTransform,   	
                                        pTransform,
                                        &hRef
                                        );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        hr = CryptXmlGetStatus( hRef, &Status );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_NOT_RESOLVED ))
        {
            //
            // Resolve the external references only.
            // The internal references will be resolved by CryptXml during CryptXmlSign
            //

            if( 0 == ( Status.dwInfoStatus & CRYPT_XML_STATUS_INTERNAL_REFERENCE ))
            {
                hr = CryptXmlGetReference(  hRef, &pRef );
                if( FAILED(hr) )
                {
                    goto CleanUp;
                }

                hr = HrSampleResolveExternalXmlReference(
                                        pRef->wszUri,
                                        &DataProvider
                                        );
                if( FAILED(hr) )
                {
                    goto CleanUp;
                }

                if( NULL == DataProvider.pfnRead )
                {
                    //
                    // Unable to open file for reading
                    //

                    hr = CRYPT_XML_E_UNRESOLVED_REFERENCE;
                    goto CleanUp;
                }

                //
                // Digest the reference
                //

                hr = CryptXmlDigestReference(
                                                hRef,
                                                0,
                                                &DataProvider
                                                );
                if( FAILED(hr) )
                {
                    goto CleanUp;
                }

                //
                // Provider must be released by the callee, which is CryptXmlDigestReference
                //
                ZeroMemory( &DataProvider, sizeof DataProvider );
            }
        }
    }

    {
        //
        // Sign 
        //
        DWORD   dwSignFlags = 0;
        CRYPT_XML_KEYINFO_PARAM KeyInfoParam = {0};
        CERT_BLOB               rgCertificate[8] = {0};
        DWORD c;

        //
        // Include the chain up to the Root
        //
        for( c=0; c<pChainContext->rgpChain[0]->cElement-1 && c<ARRAYSIZE(rgCertificate); c++ )
        {
            rgCertificate[c].pbData = pChainContext->rgpChain[0]->rgpElement[c]->pCertContext->pbCertEncoded;
            rgCertificate[c].cbData = pChainContext->rgpChain[0]->rgpElement[c]->pCertContext->cbCertEncoded;
        }

        KeyInfoParam.cCertificate = c;
        KeyInfoParam.rgCertificate = rgCertificate;

        KeyInfoParam.wszId = pPara->wszKeyInfoId;

        if( pPara->fKV )
        {
            dwSignFlags |= CRYPT_XML_SIGN_ADD_KEYVALUE;
        }

        hr = CryptXmlSign(
                                        hSig,
                                        hCryptProvOrNCryptKey,
                                        dwKeySpec,
                                        dwSignFlags,
                                        CRYPT_XML_KEYINFO_SPEC_PARAM,
                                        &KeyInfoParam,
                                        &xmlAlg_SignatureMethod,
                                        &xmlAlg_CanonicalizationMethod
                                        );
        if( FAILED(hr) )
        {
            wprintf( L"FAIL: 0x%08x CryptXmlSign\r\n", hr );
            goto CleanUp;
        }
        wprintf( L"Successfully signed and created signature.\r\n" );
    }

    {
        //
        // Encode the Signature to file
        //

        static BOOL fTRUE = TRUE;
        CRYPT_XML_PROPERTY rgEncodeProperty[] = {
            {
                //
                // This property is used to produce the declaration at the top of XML.
                //   <?xml version="1.0" encoding="utf-8" standalone="yes"?>
                //
                CRYPT_XML_PROPERTY_DOC_DECLARATION,
                &fTRUE,
                sizeof(fTRUE)
            },
        };

        hr = CryptXmlEncode(
                                        hSig,
                                        CRYPT_XML_CHARSET_UTF8,
                                        rgEncodeProperty,
                                        ARRAYSIZE(rgEncodeProperty),
                                        hFile,
                                        HrWriteXmlToFileCallback
                                        );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        wprintf( L"Successfully encoded signature to '%s'.\r\n", wszFileOut );
    }
CleanUp:

    if( INVALID_HANDLE_VALUE != hFile )
    {
        CloseHandle( hFile );
    }

    if( NULL != Encoded.pbData )
    {
        LocalFree( Encoded.pbData );
    }

    if( NULL != pChainContext )
    {
        CertFreeCertificateChain(pChainContext);
    }

    if( NULL != hSig )
    {
        CryptXmlClose( hSig );
    }

    return hr;
}
