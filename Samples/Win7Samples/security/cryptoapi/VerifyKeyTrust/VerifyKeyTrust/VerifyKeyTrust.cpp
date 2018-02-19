// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************

Title: Verifying Publisher Key of a signed binary

This sample demonstrates how Win32 applications can verify that a 
file with an Authenticode signature originates from a specific  
software publisher using WinVerifyTrust and associated helper APIs.  

The example demonstrates verifying a files signature, getting the 
publisher certificate from the signature, calculating a SHA1 hash 
of a publisher's signing key, and comparing it against a list of 
known good SHA1 hash values. 

****************************************************************/

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>

#define SHA1_HASH_LEN   20

//+-------------------------------------------------------------------------
//  SHA1 Key Identifiers of the publisher certificates
//
//  The SHA1 Key Identifier hash is obtained by executing the following
//  command for each publisher*.cer file:
//
//  >certutil -v contoso.cer
//
//  X509 Certificate:
//
//  ...
//
//
//  Key Id Hash(sha1): d9 48 12 73 f8 4e 89 90 64 47 bf 6b 85 5f b6 cb e2 3d 63 d6
//                     
//  DON'T USE: Key Id Hash(rfc-sha1):
//
//--------------------------------------------------------------------------
static const BYTE PublisherKeyList[][SHA1_HASH_LEN] = {
    //
    // The following is the sha1 key identifier for my publisher certificate
    //  CN = Contoso
    //  O = Contoso
    //  L = Redmond
    //  S = Washington
    //  C = US
    //
    {
		0xd9, 0x48, 0x12, 0x73, 0xf8, 0x4e, 0x89, 0x90, 0x64, 0x47, 
		0xbf, 0x6b, 0x85, 0x5f, 0xb6, 0xcb, 0xe2, 0x3d, 0x63, 0xd6
    },
};

#define PUBLISHER_KEY_LIST_CNT  (sizeof(PublisherKeyList) / sizeof(PublisherKeyList[0]))

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
// Checks if the leaf certificate in the chain context matches
// one of the publisher's SHA1 key identifiers.
//

static
BOOL
IsTrustedPublisherKey(
    PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    PCERT_SIMPLE_CHAIN pChain;
    PCCERT_CONTEXT pCertContext;

    //
    // Get the first simple chain
    //
    pChain = pChainContext->rgpChain[0];

    //
    // Check that the leaf certificate contains the public
    // key for one of my publisher certificates
    //
    pCertContext = pChain->rgpElement[0]->pCertContext;

    return IsTrustedKey(pCertContext, PublisherKeyList, PUBLISHER_KEY_LIST_CNT);
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

    //
    // Check for Publisher Key Trust
    //
	if (!IsTrustedPublisherKey(pProvSigner->pChainContext))
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
    printf("Usage: VerifyKeyTrust [options] <FileName>\n");
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
