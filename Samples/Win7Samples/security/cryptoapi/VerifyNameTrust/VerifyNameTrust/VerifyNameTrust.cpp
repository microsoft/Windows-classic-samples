// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Verifying Publisher Name of a Signed Binary

This sample demonstrates how Win32 applications can verify that a 
file with an Authenticode signature originates from a specific  
software publisher using WinVerifyTrust and associated helper APIs.  

The example demonstrates verifying a files signature, verifying the 
SHA1 hash of root certificate's public key, and verifying known 
fields in the publisher certificates subject name. 

****************************************************************/

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>


#define SHA1_HASH_LEN   20

//+-------------------------------------------------------------------------
//  SHA1 Key Identifiers of the publisher's root certificates
//
//  The SHA1 Key Identifier hash is obtained by executing the following
//  command for each root*.cer file:
//
//  >certutil -v contosoroot.cer
//
//  X509 Certificate:
//
//  ...
//
//
//  Key Id Hash(sha1): 25 f9 24 45 74 bc 03 89 4b 2c 5f ee 92 1b 95 5a e7 a4 9e 76
//
//  DON'T USE: Key Id Hash(rfc-sha1):
//
//--------------------------------------------------------------------------
static const BYTE RootKeyList[][SHA1_HASH_LEN] = {
    //
    // The following is the sha1 key identifier for the Contoso Root Authority
    // CN=Contoso Root Authority
	// 
    {

		0x25, 0xf9, 0x24, 0x45, 0x74, 0xbc, 0x03, 0x89, 0x4b, 0x2c, 
		0x5f, 0xee, 0x92, 0x1b, 0x95, 0x5a, 0xe7, 0xa4, 0x9e, 0x76

	},
};

#define ROOT_KEY_LIST_CNT  (sizeof(RootKeyList) / sizeof(RootKeyList[0]))


//+-------------------------------------------------------------------------
//  Subject Name Attributes Used to Identify My Publisher Certificates
//--------------------------------------------------------------------------
static const LPCSTR PublisherAttributeObjId[] = {
    // 0 - O=
    szOID_ORGANIZATION_NAME,

    // 1 - L=
    szOID_LOCALITY_NAME,

    // 2 - S=
    szOID_STATE_OR_PROVINCE_NAME,

    // 3 - C=
    szOID_COUNTRY_NAME,
};


#define PUBLISHER_ATTR_CNT  (sizeof(PublisherAttributeObjId) / sizeof(PublisherAttributeObjId[0]))

//+-------------------------------------------------------------------------
//  Subject name attributes of my publisher certificates
//
//  The name attributes are obtained by executing the following
//  command for each publisher*.cer file:
//
//  >certutil -v contoso.cer
//
//  X509 Certificate:
//
//  ...
//
//  Subject:
//    CN=Contoso
//    O=Contoso
//    L=Redmond
//    S=Washington
//    C=US
//
//--------------------------------------------------------------------------
static const LPCWSTR PublisherNameList[][PUBLISHER_ATTR_CNT] = {
    {
        // 0 - O=
        L"Contoso",

        // 1 - L=
        L"Redmond",

        // 2 - S=
        L"Washington",

        // 3 - C=
        L"US"
    },
};

#define PUBLISHER_NAME_LIST_CNT  (sizeof(PublisherNameList) / sizeof(PublisherNameList[0]))

//
// Checks if the certificate's public key matches one of the SHA1 key identifiers
// in the list.
//

static
BOOL
IsTrustedKey(
    PCCERT_CONTEXT pCertContext,
    const BYTE KeyList[][SHA1_HASH_LEN],
    DWORD KeyCount
    )
{
    BYTE rgbKeyId[SHA1_HASH_LEN];
    DWORD cbKeyId;

    cbKeyId = SHA1_HASH_LEN;
    if (!CryptHashPublicKeyInfo(
            NULL,               // hCryptProv
            CALG_SHA1,
            0,                  // dwFlags
            X509_ASN_ENCODING,
            &pCertContext->pCertInfo->SubjectPublicKeyInfo,
            rgbKeyId,
            &cbKeyId) || SHA1_HASH_LEN != cbKeyId)
    {
        return FALSE;
    }

    for (DWORD i = 0; i < KeyCount; i++)
    {
        if (0 == memcmp(KeyList[i], rgbKeyId, SHA1_HASH_LEN))
        {
            return TRUE;
        }
    }

    return FALSE;
}


//
// Checks if the root certificate in the chain context matches
// one of the publisher root's SHA1 key identifiers. This ensures
// that the publishers certificate is issued from a trusted 
// certification authority
//

static
BOOL
IsTrustedRootKey(
    PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    PCERT_SIMPLE_CHAIN pChain;
    DWORD cChainElement;
    PCCERT_CONTEXT pCertContext;

    //
    // Get the first simple chain and its element count.
    //

    pChain = pChainContext->rgpChain[0];
    cChainElement = pChain->cElement;

    //
    // Check that the top level certificate contains the public
    // key for one of my roots
    //

    pCertContext = pChain->rgpElement[cChainElement - 1]->pCertContext;

    return IsTrustedKey(pCertContext, RootKeyList, ROOT_KEY_LIST_CNT);
}

//
// Checks if the leaf certificate in the chain context matches
// one of the publisher's list of subject name attributes.
//

static
BOOL
IsTrustedPublisherName(
    PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    PCERT_SIMPLE_CHAIN pChain;
    PCCERT_CONTEXT pCertContext;

    //
    // Get the publisher's certificate from the chain context
    //

    pChain = pChainContext->rgpChain[0];
    pCertContext = pChain->rgpElement[0]->pCertContext;

    //
    // Loop through the list of publisher subject names to be matched
    //

    for (DWORD i = 0; i < PUBLISHER_NAME_LIST_CNT; i++)
    {
        BOOL trusted = TRUE;

        //
        // Loop through the subject name attributes to be matched.
        // For example, O= ; L= ; S= ; C= ;
        //

        for (DWORD j = 0; trusted && j < PUBLISHER_ATTR_CNT; j++)
        {
            LPWSTR AttrString = NULL;
            DWORD AttrStringLength;


            //
            // First pass call to get the length of the subject name attribute.
            // Note, the returned length includes the NULL terminator.
            //

            AttrStringLength = CertGetNameStringW(
                pCertContext,
                CERT_NAME_ATTR_TYPE,
                0,                      // dwFlags
                (void *) PublisherAttributeObjId[j],
                NULL,                   // AttrString
                0);                     // AttrStringLength

            if (AttrStringLength <= 1)
            {
                //
                // A matching certificate must have all of the attributes
                //

                return FALSE;
            }


            AttrString = (LPWSTR) LocalAlloc(
                LPTR,
                AttrStringLength * sizeof(WCHAR));
            if (AttrString == NULL)
            {
                return FALSE;
            }

            //
            // Second pass call to get the subject name attribute
            //

            AttrStringLength = CertGetNameStringW(
                pCertContext,
                CERT_NAME_ATTR_TYPE,
                0,                      // dwFlags
                (void *) PublisherAttributeObjId[j],
                AttrString,
                AttrStringLength);

            if (AttrStringLength <= 1 ||
                0 != wcscmp(AttrString, PublisherNameList[i][j]))
            {
                // The subject name attribute doesn't match
                trusted = FALSE;
            }

            LocalFree(AttrString);
        }

        if (trusted)
        {
            // All the subject name attributes match
            return TRUE;
        }
    }

    return FALSE;
}

//
// Checks that the file is authenticode signed by the publisher
//

BOOL
IsFilePublisherTrusted(
    LPCWSTR pwszFileName
    )
{
    BOOL trusted = FALSE;
    DWORD lastError;
    GUID wvtProvGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    //
    // Initialize structure for WinVerifyTrust call
    //

	WINTRUST_DATA wtd = { 0 };
	WINTRUST_FILE_INFO wtfi = { 0 };

    wtd.cbStruct = sizeof(WINTRUST_DATA);
    wtd.pPolicyCallbackData = NULL;
    wtd.pSIPClientData = NULL;
    wtd.dwUIChoice = WTD_UI_NONE;
    wtd.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
    wtd.dwUnionChoice = WTD_CHOICE_FILE;
    wtd.pFile = &wtfi;
    wtd.dwStateAction = WTD_STATEACTION_VERIFY;
    wtd.hWVTStateData = NULL;
    wtd.pwszURLReference = NULL;
    wtd.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;

    wtfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
    wtfi.pcwszFilePath = pwszFileName;
    wtfi.hFile = NULL;
    wtfi.pgKnownSubject = NULL;

    //
    // Check the file's Authenticode signature
    //

    if (S_OK != WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &wvtProvGuid, &wtd))
    {
        lastError = GetLastError();
        goto Cleanup;
    }

    //
    // Get provider data
    //

    CRYPT_PROVIDER_DATA* pProvData = WTHelperProvDataFromStateData(wtd.hWVTStateData);
    if (NULL == pProvData)
    {
        lastError = GetLastError();
        goto Cleanup;
    }

    //
    // Get the signer
    //

    CRYPT_PROVIDER_SGNR* pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);
    if (NULL == pProvSigner)
    {
        lastError = GetLastError();
        goto Cleanup;
    }

    if (!IsTrustedRootKey(pProvSigner->pChainContext))
    {
        goto Cleanup;
    }

    if (!IsTrustedPublisherName(pProvSigner->pChainContext))
    {
        goto Cleanup;
    }

    //
    // If we made it this far, we can trust the file
    //

    trusted = TRUE;

Cleanup:
    //
    // Close the previously-opened state data handle
    //

    wtd.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &wvtProvGuid, &wtd);

    return trusted;
}


// --------------------------------------------------------------------------

//
// Command line test tool to call the above IsFilePublisherTrusted() function.
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void Usage(void)
{
    printf("Usage: VerifyNameTrust [options] <FileName>\n");
    printf("Options are:\n");
    printf("  -h                    - This message\n");
    printf("\n");
}

int _cdecl wmain(int argc, __in_ecount(argc) wchar_t * argv[]) 
{
    int status;
    BOOL trusted;
    LPCWSTR pwszFileName = NULL;    // Not allocated

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
                case 'h':
                default:
                    goto BadUsage;
            }
        } else {
            if (pwszFileName == NULL)
                pwszFileName = argv[0];
            else {
                printf("Too many arguments\n");
                goto BadUsage;
            }
        }
    }

    if (pwszFileName == NULL) {
        printf("missing FileName \n");
        goto BadUsage;
    }

    wprintf(L"command line: %s\n", GetCommandLineW());

    trusted = IsFilePublisherTrusted(pwszFileName);

    if (trusted)
    {
        printf("Passed :: Publisher Trusted File\n");
    }
    else
    {
        printf("Failed :: NOT PUBLISHER TRUSTED File\n");
    }

    status = 0;

CommonReturn:

    return status;
ErrorReturn:
    status = -1;
    printf("Failed\n");
    goto CommonReturn;

BadUsage:
    Usage();
    goto ErrorReturn;
    
}


