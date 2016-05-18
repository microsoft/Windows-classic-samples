// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"

/*****************************************************************************
 HrVerify

 This sample demontrates XML signature verification

*****************************************************************************/
HRESULT
HrVerify(
    LPCWSTR         wszFileIn
    )
{
    HRESULT     hr = S_FALSE;
    HCRYPTXML   hDoc = NULL;
    ULONG       i;
    const CRYPT_XML_DOC_CTXT  *pDoc = NULL;
    const CRYPT_XML_SIGNATURE *pSig = NULL;
    const CRYPT_XML_REFERENCE *pRef = NULL;

    CRYPT_XML_STATUS    Status = {0};

    BCRYPT_KEY_HANDLE   hKey = NULL;
    BCRYPT_KEY_HANDLE   hKeyAlloc = NULL;   // BCryptDestroyKey
    PCCERT_CONTEXT      pCert = NULL;

    CRYPT_XML_DATA_PROVIDER DataProvider = {0};

    CRYPT_XML_BLOB   Encoded = { CRYPT_XML_CHARSET_AUTO, 0, NULL };

    hr = HrLoadFile(
                                        wszFileIn,
                                        &Encoded.pbData,
                                        &Encoded.cbData
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    hr = CryptXmlOpenToDecode(
                                        NULL,               // no custom transforms
                                        0,
                                        NULL,
                                        0,
                                        &Encoded,
                                        &hDoc
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    hr = CryptXmlGetDocContext( 
                                        hDoc, 
                                        &pDoc 
                                        );
    if( FAILED(hr) )
    {
        goto CleanUp;
    }

    if( pDoc->cSignature < 1 )
    {
        wprintf( L"WARNING: The document contains no Signature element.\r\n" );

        // No signatures found
        hr = S_FALSE;
        goto CleanUp;
    }

    for( i=0; i<pDoc->cSignature; i++ )
    {
        hKey = NULL;
        if( NULL == hKeyAlloc )
        {
            BCryptDestroyKey( hKeyAlloc );
            hKeyAlloc = NULL;
        }

        //
        // Find Signer's certificate
        //

        for( ULONG ki=0; ki<pDoc->rgpSignature[i]->pKeyInfo->cKeyInfo && NULL == hKey; ki++ )
        {
            if( CRYPT_XML_KEYINFO_TYPE_X509DATA ==
                pDoc->rgpSignature[i]->pKeyInfo->rgKeyInfo[ki].dwType )
            {
                for( ULONG x=0; x<pDoc->rgpSignature[i]->pKeyInfo->rgKeyInfo[ki].X509Data.cX509Data; x++ )
                {
                    CRYPT_XML_X509DATA_ITEM *pX = &pDoc->rgpSignature[i]->pKeyInfo->rgKeyInfo[ki].X509Data.rgX509Data[x];
                    if( CRYPT_XML_X509DATA_TYPE_CERTIFICATE == pX->dwType )
                    {
                        //
                        // SAMPLE: Just assume that the first cert is the one ...
                        //         In production code, implement the full logic
                        //          to find the signer's cert, from a set of multiple X.509 data elements
                        //
                        
                        pCert = CertCreateCertificateContext( 
                                        X509_ASN_ENCODING,
                                        pX->Certificate.pbData,
                                        pX->Certificate.cbData
                                        );

                        if( NULL != pCert )
                        {
                            if( CryptImportPublicKeyInfoEx2(
                                        X509_ASN_ENCODING,
                                        &pCert->pCertInfo->SubjectPublicKeyInfo,
                                        CRYPT_OID_INFO_PUBKEY_SIGN_KEY_FLAG,
                                        NULL,
                                        &hKeyAlloc 
                                        ))
                            {
                                hKey = hKeyAlloc;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if( NULL == hKey )
        {
            wprintf( L"ERROR: Unable to find signer's key to verify signature [%d] Id='%s'\r\n",  
                        i,
                        pDoc->rgpSignature[i]->wszId 
                        );

            hr = CRYPT_XML_E_SIGNER;
            goto CleanUp;
        }

        //
        // SAMPLE: Verify Signer's Trust
        //
        {
            //
            // TODO: Use CertGetCertificateChain() to build the chain and verify the trust.
            //
            
            //
            // TODO: Accept only strong cryptographic algorithms and key strengths.
            // Also, place max key length on key size to avoid Denial of Service (DoS)
            // attacks on verification key operations. 
            //
        }

        hr = CryptXmlVerifySignature(
                                        pDoc->rgpSignature[i]->hSignature,
                                        hKey,
                                        0
                                        );
        if( FAILED(hr) )
        {
            wprintf( L"FAIL: CryptXmlVerifySignature() returned 0x%08x error on signature [%d] Id='%s'\r\n",  
                        hr,
                        i,
                        pDoc->rgpSignature[i]->wszId 
                        );
            goto CleanUp;
        }

        wprintf( L"Signature Value on signature [%d] Id='%s' is valid.\r\n",  
                        i,
                        pDoc->rgpSignature[i]->wszId 
                        );

        //
        // Verify References
        //

        hr = CryptXmlGetSignature(
                                        pDoc->rgpSignature[i]->hSignature,
                                        &pSig
                                        );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        for( ULONG r=0; r<pSig->SignedInfo.cReference; r++ )
        {
            hr = CryptXmlGetReference(
                                        pSig->SignedInfo.rgpReference[r]->hReference,
                                        &pRef
                                        );
            if( FAILED(hr) )
            {
                goto CleanUp;
            }

            hr = CryptXmlGetStatus(
                                        pSig->SignedInfo.rgpReference[r]->hReference,
                                        &Status
                                        );
            if( FAILED(hr) )
            {
                goto CleanUp;
            }

            if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_NOT_RESOLVED ))
            {
                //
                // Resolve the external references only.
                // The internal references was resolved by CryptXml during CryptXmlVerifySignature
                //

                if( 0 == ( Status.dwInfoStatus & CRYPT_XML_STATUS_INTERNAL_REFERENCE ))
                {
                    //
                    // TODO:  Verify the scope of the Reference Target URI prior to resolving 
                    //

                    hr = HrSampleResolveExternalXmlReference(
                                        pRef->wszUri,
                                        &DataProvider
                                        );
                    if( FAILED(hr) )
                    {
                        goto CleanUp;
                    }
                }

                if( NULL == DataProvider.pfnRead )
                {
                    hr = CRYPT_XML_E_UNRESOLVED_REFERENCE;
                    goto CleanUp;
                }

                //
                // Provider must be released by the callee
                //

                hr = CryptXmlDigestReference(
                                        pSig->SignedInfo.rgpReference[r]->hReference,
                                        0,
                                        &DataProvider
                                        );

                ZeroMemory( &DataProvider, sizeof DataProvider );

                //
                // Get the status to check it again
                //

                hr = CryptXmlGetStatus(
                                        pSig->SignedInfo.rgpReference[r]->hReference,
                                        &Status
                                        );
            }

            if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_DIGEST_INVALID ))
            {
                wprintf( L"Digest Value on reference[%d] Id='%s' is not valid.\r\n",  
                                        r,
                                        pSig->SignedInfo.rgpReference[r]->wszId );
            }
            else
            {
                if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_NOT_RESOLVED ))
                {
                    wprintf( L"Reference[%d] Id='%s' is not resolved\r\n",  
                                        r,
                                        pSig->SignedInfo.rgpReference[r]->wszId );
                }
                else
                {
                    wprintf( L"Digest Value on reference[%d] Id='%s' is valid\r\n",  
                                        r,
                                        pSig->SignedInfo.rgpReference[r]->wszId );
                }
            }
        }

        //
        // Average status on the Signature
        //

        hr = CryptXmlGetStatus( pDoc->rgpSignature[i]->hSignature, &Status );
        if( FAILED(hr) )
        {
            goto CleanUp;
        }

        if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_DIGEST_INVALID ))
        {
            hr = CRYPT_XML_E_INVALID_DIGEST;
            goto CleanUp;
        }

        if( 0 != ( Status.dwErrorStatus & CRYPT_XML_STATUS_ERROR_NOT_RESOLVED ))
        {
            hr = CRYPT_XML_E_UNRESOLVED_REFERENCE;
            goto CleanUp;
        }
    }

    hr = S_OK;

CleanUp:

    if( NULL != Encoded.pbData )
    {
        LocalFree( Encoded.pbData );
    }

    if( NULL == hKeyAlloc )
    {
        BCryptDestroyKey( hKeyAlloc );
    }

    if( NULL != pCert )
    {
        CertFreeCertificateContext( pCert );
    }

    return hr;
}
