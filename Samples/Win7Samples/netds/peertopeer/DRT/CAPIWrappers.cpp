// CAPIWrappers.cpp - Functions for dealing with certificates.  

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif

#include "CAPIWrappers.h"

#pragma comment(lib, "crypt32")
#pragma comment(lib, "rpcrt4")

#ifndef USES
#define USES(x) (x = x)
#endif

#define HRESULT_FROM_RPCSTATUS(x) \
        (((x < 0) || (x == RPC_S_OK)) ?                                     \
        (HRESULT)x :                \
        (HRESULT) (((x) & 0x0000FFFF) | (FACILITY_RPC<< 16) | 0x80000000))

#define SECOND_IN_FILETIME (10i64 * 1000 * 1000)



const DWORD SIXTYFOUR_K = 64 * 1024;
const DWORD SIXTEEN_K = 16 * 1024;
const DWORD ONE_K = 1024;

BYTE s_keyDataBuf[SIXTEEN_K] = {0};
BYTE s_certBuf[SIXTEEN_K] = {0};
BYTE s_fileBuf[SIXTYFOUR_K] = {0};

/****************************************************************************++

  Description :

    This function creates a well known sid using User domain. CreateWellKnownSid requires
    domain sid to be provided to generate such sids. This function first gets the domain sid
    out of the user information in the token and then generate a well known sid.

  Arguments:

    hToken - [supplies] The token for which sid has to be generated
    sidType - [supplies] The type of well known sid
    pSid - [receives] The newly create sid
    pdwSidSize - [Supplies/Receives] The size of the memory allocated for ppSid

  Returns:

    Errors returned by GetTokenInformation
    Errors returned by CreateWellKnownSid
    E_OUTOFMEMORY In case there is not enough memory
    Errors returned by GetWindowsAccountDomainSid
--***************************************************************************/
HRESULT
CreateWellKnownSidForAccount(
    __in_opt    HANDLE                  hToken,
    __in        WELL_KNOWN_SID_TYPE     sidType,
    __out       PSID                    pSid,
    __inout     DWORD                   * pdwSidSize)
{
    HRESULT         hr = S_OK;
    TOKEN_USER      * pUserToken = NULL;
    DWORD           dwTokenLen = 0;
    PBYTE           pDomainSid[SECURITY_MAX_SID_SIZE];
    DWORD           dwSidSize = SECURITY_MAX_SID_SIZE;


    //
    // Get the TokenUser, use this TokenUser to generate a well-known sid that requires a domain
    //
    if (!GetTokenInformation(
        hToken,
        TokenUser,
        NULL,
        dwTokenLen,
        &dwTokenLen))
    {
        DWORD   err = GetLastError();
        if (err != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = HRESULT_FROM_WIN32(err);
            goto cleanup;
        }
    }

    pUserToken = (TOKEN_USER *) new BYTE[dwTokenLen];
    if (pUserToken == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    if (!GetTokenInformation(
        hToken,
        TokenUser,
        pUserToken,
        dwTokenLen,
        &dwTokenLen))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    //
    // Now get the domain sid from the TokenUser
    //
    if (!GetWindowsAccountDomainSid(
            pUserToken->User.Sid,
            (PSID) pDomainSid,
            &dwSidSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    if(!CreateWellKnownSid(
        sidType,
        pDomainSid,
        pSid,
        pdwSidSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

cleanup:
    delete [] pUserToken;

    return hr;
}


/****************************************************************************++

Routine Description:

    Verifies whether specified well-known SID is in the current user token

Arguments:

    sid - one of the WELL_KNOWN_SID_TYPE consts
    hToken - Optional the token for which we want to test membership
    pfMember - [Receives] TRUE if specified sid is a member of the user token, FALSE otherwise

Notes:

    -

Return Value:

    Errors returned by CreateWellKnownSid
    Errors returned by CheckTokenMembership
--*****************************************************************************/
HRESULT
IsMemberOf(
    __in      WELL_KNOWN_SID_TYPE     sid,
    __in_opt  HANDLE                  hToken,
    __out     BOOL                    * pfMember)
{
    HRESULT     hr = S_OK;
    BOOL        fMember = FALSE;
    BYTE        pSID[SECURITY_MAX_SID_SIZE] = {0};
    DWORD       dwSIDSize = sizeof(pSID);

    //
    // create SID for the authenticated users
    //
    if (!CreateWellKnownSid(
            sid,
            NULL,                    // not a domain sid
            (SID*)pSID,
            &dwSIDSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr) && (hr != E_INVALIDARG))
        {
            goto Cleanup;
        }

        //
        // In case of invalid-arg we might need to provide the domain, so create well known sid for domain
        //
        hr = CreateWellKnownSidForAccount(hToken, sid, pSID, &dwSIDSize);
        if (hr == HRESULT_FROM_WIN32(ERROR_NON_ACCOUNT_SID))
        {
            //
            // If it is a non account sid (for example Local Service). Ignore the error.
            //
            hr = S_OK;
            fMember = FALSE;
            goto Cleanup;
        }
        else if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    //
    // check whether token has this sid
    //
    if (!CheckTokenMembership(hToken,
                                (SID*)pSID,    // sid for the authenticated user
                                &fMember))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        // just to be on the safe side (as we don't know that CheckTokenMembership
        // does not modify fAuthenticated in case of error)
        fMember = FALSE;
        if (hr == E_ACCESSDENIED && hToken == NULL)
        {
            // unable to query the thread token. Open as self and try again
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
            {
                if (CheckTokenMembership(hToken, (SID*)pSID, &fMember))
                {
                    hr = S_OK;
                }
                else
                {
                    // stick with the original error code, but ensure that fMember is correct
                    fMember = FALSE;
                }
                CloseHandle(hToken);
            }
        }
        goto Cleanup;
    }

Cleanup:
    *pfMember = fMember;
    return hr;
}


// helper used by CreateCryptProv
HRESULT
IsServiceAccount(
    OUT BOOL    * pfMember)
{
    HRESULT hr = S_OK;
    BOOL    fMember = FALSE;

    hr = IsMemberOf(WinLocalServiceSid, NULL, &fMember);
    if (FAILED(hr) || fMember)
    {
        goto Cleanup;
    }

    hr = IsMemberOf(WinLocalSystemSid, NULL, &fMember);
    if (FAILED(hr) || fMember)
    {
        goto Cleanup;
    }

    hr = IsMemberOf(WinNetworkServiceSid, NULL, &fMember);
    if (FAILED(hr) || fMember)
    {
        goto Cleanup;
    }

Cleanup:

    *pfMember = fMember;
    return hr;
}

/****************************************************************************++

Routine Description:

    Deletes the key container and the keys

Arguments:

    pwzContainer -

Notes:

    -

Return Value:

    - S_OK

      - or -

    - no other errors are expected

--*****************************************************************************/
HRESULT
DeleteKeys(
    IN      PCWSTR       pwzContainer)
{
    HRESULT     hr = S_OK;
    HCRYPTPROV  hCryptProv = NULL;
    BOOL        fServiceAccount = FALSE;

    hr = IsServiceAccount(&fServiceAccount);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    //
    // this is the most counter-intuitive API that i have seen in my life
    // in order to delete the contanier and all the keys in it, i have to call CryptAcquireContext
    //
    if (!CryptAcquireContextW(&hCryptProv,
                               pwzContainer,
                               NULL,
                               DEFAULT_PROV_TYPE,
                               fServiceAccount ?
                                   (CRYPT_DELETEKEYSET | CRYPT_MACHINE_KEYSET) :
                                   (CRYPT_DELETEKEYSET)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

Cleanup:
    return hr;
}

/****************************************************************************++

Routine Description:

    Wrapper fro CryptDestroyKey

Arguments:

    hKey - handle to the key to destroy

Notes:

    -

Return Value:

    - VOID

--*****************************************************************************/
VOID
DestroyKey(
    IN      HCRYPTKEY       hKey)
{
    if (NULL != hKey)
    {
        //
        // this is quite counter-intuitive API
        // CryptDestroyKey just releases the handle to the key, but only in case of
        // private/public key pairs.
        //
        if (!CryptDestroyKey(hKey))
        {
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            USES(hr);
            //
            // should never happen, unless handle is invalid
            //
        }
    }
}

/****************************************************************************++

Routine Description:

    Releases the handle to the CSP

Arguments:

    hCryptProv - handle to the CSP

Notes:

    - handles the NULL CSP gracefully

Return Value:

    - VOID

--*****************************************************************************/
VOID
ReleaseCryptProv(
    IN      HCRYPTPROV      hCryptProv)
{
    if (NULL != hCryptProv)
    {
        if (!CryptReleaseContext(hCryptProv, 0))
        {
            //
            // one reason why this could fail (at least it failed a couple of times already) i s
            // that some certifcate store was opened using this CSP, but
            // CERT_STORE_NO_CRYPT_RELEASE_FLAG was not specified, so that
            // when cert store is released the provider is released as well.
            //
            // verify that all stores that were ever used, specify this flag
            //

            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            USES(hr);
        }
    }
}




/****************************************************************************++

Routine Description:

    Creates a handle to the CSP

Arguments:

    pwzContainerName - name of the container to be created. if NULL, GUID is generated
                      for the name of the container

    fCreateNewKeys   - forces new keys to be created

    phCryptProv      - pointer to the location, where handle should be returned

Notes:

    -

Return Value:

    - S_OK

      - or -

    - CAPI error returned by CryptAcquireContextW

--*****************************************************************************/
HRESULT
CreateCryptProv(
    IN      PCWSTR          pwzContainerName,
    IN      BOOL            fCreateNewKeys,
    OUT     HCRYPTPROV*     phCryptProv)
{

    HRESULT         hr = S_OK;
    HCRYPTKEY       hKey = NULL;
    RPC_STATUS      status =  RPC_S_OK;
    BOOL            fCreatedContainer = FALSE;
    WCHAR*          pwzNewContainerName = NULL;

    *phCryptProv = NULL;

    if (NULL == pwzContainerName)
    {
         UUID    uuid;
         BOOL   fServiceAccount = FALSE;

        //
        // generate container name from the UUID
        //
        status = UuidCreate(&uuid);
        hr = HRESULT_FROM_RPCSTATUS(status);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        status = UuidToStringW(&uuid, (unsigned short**)&pwzNewContainerName);
        hr = HRESULT_FROM_RPCSTATUS(status);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        pwzContainerName = pwzNewContainerName;

        hr = IsServiceAccount(&fServiceAccount);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        //
        // open the clean key container
        //
        // note: CRYPT_NEW_KEYSET is not creating new keys, it just
        // creates new key container. duh.
        //
        if (!CryptAcquireContextW(phCryptProv,
                                pwzNewContainerName,
                                NULL,               // default provider name
                                DEFAULT_PROV_TYPE,
                                fServiceAccount ?
                                    (CRYPT_SILENT | CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET) :
                                    (CRYPT_SILENT | CRYPT_NEWKEYSET)))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            //
            // we are seeing that CryptAcquireContextW returns NTE_FAIL under low
            // memory condition, so we just mask the error
            //
            if (NTE_FAIL == hr)
            {
                hr = E_OUTOFMEMORY;
            }

            goto Cleanup;
        }

        fCreatedContainer = TRUE;

    }
    else
    {
        BOOL    fServiceAccount = FALSE;

        hr = IsServiceAccount(&fServiceAccount);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        //
        // open the provider first, create the keys too
        //
        if (!CryptAcquireContextW(phCryptProv,
                            pwzContainerName,
                            NULL,               // default provider name
                            DEFAULT_PROV_TYPE,
                            fServiceAccount ?
                                (CRYPT_SILENT | CRYPT_MACHINE_KEYSET) :
                                (CRYPT_SILENT)))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            //
            // we are seeing that CryptAcquireContextW returns NTE_FAIL under low
            // memory condition, so we just mask the error
            //
            if (NTE_FAIL == hr)
            {
                hr = E_OUTOFMEMORY;
            }

            goto Cleanup;
        }
    }

    if (fCreateNewKeys)
    {
        //
        // make sure keys exist
        //
        if (!CryptGetUserKey(*phCryptProv,
                            DEFAULT_KEY_SPEC,
                            &hKey))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            // if key does not exist, create it
            if (HRESULT_FROM_WIN32((unsigned long)NTE_NO_KEY) == hr)
            {
                hr = S_OK;

                if (!CryptGenKey(*phCryptProv,
                                  DEFAULT_KEY_SPEC,
                                  CRYPT_EXPORTABLE,
                                  &hKey))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());

                    //
                    // we are seeing that CryptGenKey returns ERROR_CANTOPEN under low
                    // memory condition, so we just mask the error
                    //
                    if (HRESULT_FROM_WIN32(ERROR_CANTOPEN) == hr)
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    goto Cleanup;
                }

            }
            else
            {
                // failed to get user key by some misterious reason, so bail out
                goto Cleanup;
            }
        }
    }

Cleanup:

    DestroyKey(hKey);

    if (FAILED(hr))
    {

        //
        // release the context
        //
        ReleaseCryptProv(*phCryptProv);
        *phCryptProv = NULL;

        //
        // delete the keys, if we created them
        //
        if (fCreatedContainer)
        {
            DeleteKeys(pwzContainerName);
        }
    }

    if (NULL != pwzNewContainerName)
    {
        // this always returns RPC_S_OK
        status = RpcStringFreeW((unsigned short**)&pwzNewContainerName);
        USES(status);
    }

    return hr;
}

/****************************************************************************++

Routine Description:

    Retrieves the name of the CSP container.

Arguments:

    hCryptProv      - handle to the CSP

    pcChars         - count of chars in the buffer on input.  count of chars used on return

    pwzContainerName - pointer to output buffer

Notes:

    -

Return Value:

    - S_OK

      - or -

    - NTE_BAD_UID, if hCryptProv handle is not valid

--*****************************************************************************/
HRESULT
GetContainerName(
    IN      HCRYPTPROV  hCryptProv,
    __inout ULONG*      pcChars,
    __out_ecount_opt(*pcChars) LPWSTR   pwzContainerName)
{
    HRESULT     hr = S_OK;
    CHAR *      pszBuf = new CHAR[*pcChars];
    ULONG       cbBufSize = sizeof(CHAR) *(*pcChars);

    if (NULL == pszBuf)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    //
    // get the name of the key container
    //
    if (!CryptGetProvParam(hCryptProv,
                          PP_CONTAINER,
                          (BYTE*)pszBuf,
                          &cbBufSize,
                          0))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        // if buffer is not sufficiently large, just return with this error
        if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr)
        {
            // if buffer was not sufficiently large return the number of characters needed
            *pcChars = cbBufSize / sizeof(CHAR);
            goto Cleanup;
        }
        else
        {
            goto Cleanup;
        }
    }

    //
    // convert the string to the wide character, since that's what needed for the key info
    //
    if (0 == MultiByteToWideChar(CP_ACP,
                               0,                   // dwFlags
                               pszBuf,
                               -1,                  // calculate the length
                               pwzContainerName,
                               *pcChars))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

Cleanup:

    if (NULL != pszBuf)
    {
        delete [] pszBuf;
    }

    return hr;
}

// Read a single cert from a file
HRESULT ReadCertFromFile(LPCWSTR pwzFileName, CERT_CONTEXT** ppCert, HCRYPTPROV* phCryptProv)
{
    BOOL bRet = FALSE;
    FILE* pFile = NULL;
    errno_t err;
    
    // open cert file for local cert
    err = _wfopen_s(&pFile, pwzFileName, L"rb");
    if (err)
    {
        return CRYPT_E_FILE_ERROR;
    }

    // read local cert into *ppLocalCert, allocating memory
    fread(s_fileBuf, sizeof(s_fileBuf), 1, pFile);
    fclose(pFile);

    CRYPT_DATA_BLOB blob;
    blob.cbData = sizeof(s_fileBuf);
    blob.pbData = s_fileBuf;
    HCERTSTORE hCertStore = PFXImportCertStore(&blob, L"DRT Rocks!", CRYPT_EXPORTABLE);
    if (NULL == hCertStore)
        return HRESULT_FROM_WIN32(GetLastError());

    // TODO: does this have to be a c style cast? I get compile errors if I try reinterpret or static cast
    // the first cert is always the leaf cert (since we encoded it that way)
    CERT_CONTEXT* pCertContext = (CERT_CONTEXT*)CertEnumCertificatesInStore(hCertStore, NULL);
    if (NULL == pCertContext)
        return HRESULT_FROM_WIN32(GetLastError());

    // retreive the crypt provider which has the private key for this certificate
    DWORD dwKeySpec = 0;
    HCRYPTPROV hCryptProv = NULL;
    bRet = CryptAcquireCertificatePrivateKey(pCertContext,
        CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
        NULL, &hCryptProv, &dwKeySpec, NULL);
    if (!bRet)
            return HRESULT_FROM_WIN32(GetLastError());

    // make sure provider stays around for duration of the test run. We need hCryptProv of root cert to sign local certs
    CryptContextAddRef(hCryptProv, NULL, 0);

    // everything succeeded, safe to set outparam
    *ppCert = pCertContext;
    if (NULL != phCryptProv)
        *phCryptProv = hCryptProv;
    
    return S_OK;
}


// helper function to write the cert store out to a file
HRESULT WriteStoreToFile(__in HCERTSTORE hCertStore, __in PCWSTR pwzFileName)
{
    BOOL bRet = FALSE;
    FILE* pFile = NULL;
    errno_t err;

    CRYPT_DATA_BLOB blob = { sizeof(s_fileBuf), s_fileBuf};
    bRet = PFXExportCertStore(hCertStore, &blob, L"DRT Rocks!", EXPORT_PRIVATE_KEYS);
    if (!bRet)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    err = _wfopen_s(&pFile, pwzFileName, L"wb");
    if (err)
    {
        return CRYPT_E_FILE_ERROR;
    }
    
    fwrite(blob.pbData, blob.cbData, 1, pFile);
    fclose(pFile);

    return S_OK;
}


// helper function used by make certs to encode a name for storage in a cert (modified from drt\test\drtcert\main.cpp)
HRESULT EncodeName(__in PCWSTR pwzName, DWORD* pcbEncodedName, BYTE* pbEncodedName)
{
    CERT_NAME_INFO nameInfo = {0};
    CERT_RDN rdn = {0};
    CERT_RDN_ATTR rdnAttr = {0};

    nameInfo.cRDN = 1;
    nameInfo.rgRDN = &rdn;
    rdn.cRDNAttr = 1;
    rdn.rgRDNAttr = &rdnAttr;
    rdnAttr.dwValueType = CERT_RDN_UNICODE_STRING;
    rdnAttr.pszObjId = szOID_COMMON_NAME;
    rdnAttr.Value.pbData = (BYTE*)pwzName;
    rdnAttr.Value.cbData = sizeof(WCHAR) * (DWORD)(wcslen(pwzName) + 1);

    if (!CryptEncodeObject(DEFAULT_ENCODING, X509_NAME, &nameInfo, pbEncodedName, pcbEncodedName))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

// manufacture a single cert and export it to a file
HRESULT MakeAndExportACert(PCWSTR pwzSignerName, PCWSTR pwzCertName, PCWSTR pwzFileName, HCERTSTORE hCertStore,
                                                        HCRYPTPROV hCryptProvSigner, HCRYPTPROV hCryptProvThisCert,
                                                        CERT_CONTEXT* pIssuerCertContext)
{
    HRESULT hr = S_OK;
    BOOL     bRet = FALSE;
    BYTE byteBuf1[ONE_K] = {0};
    BYTE byteBuf2[ONE_K] = {0};
    DWORD cSignerName = sizeof(byteBuf1);
    BYTE* pbSignerName = byteBuf1;
    DWORD cCertName = sizeof(byteBuf2);
    BYTE* pbCertName = byteBuf2;    
    CERT_INFO certInfo = {0};
    BYTE serialNumberBuf[16];
    ULONGLONG ullTime = 0;
    XCERT_CONTEXT pCertContext;
    ULONG cchContainer = 512;
    WCHAR wzContainer[512];
    CRYPT_KEY_PROV_INFO keyInfo = {0};    
    
   
    // encode the names for use in a cert
    hr = EncodeName(pwzSignerName, &cSignerName, pbSignerName);
    if (FAILED(hr))
        return hr;
    
    hr = EncodeName(pwzCertName, &cCertName, pbCertName);
    if (FAILED(hr))
        return hr;
    
    // first retrieve the public key from the hCryptProv (which abstracts the key pair)
    DWORD dwSize = sizeof(s_keyDataBuf);
    bRet = CryptExportPublicKeyInfo(hCryptProvThisCert, DEFAULT_KEY_SPEC, DEFAULT_ENCODING,
                                                          reinterpret_cast<CERT_PUBLIC_KEY_INFO*>(s_keyDataBuf), &dwSize);
    if (FALSE == bRet)
        return HRESULT_FROM_WIN32(GetLastError());

    // set the cert properties
    certInfo.dwVersion = CERT_V3;
    certInfo.SerialNumber.cbData = sizeof(serialNumberBuf);
    certInfo.SerialNumber.pbData = serialNumberBuf;
    certInfo.SignatureAlgorithm.pszObjId = DEFAULT_ALGORITHM;
    GetSystemTimeAsFileTime((FILETIME*)&ullTime);
    ullTime -= SECOND_IN_FILETIME * 60 * 60 * 24;
    CopyMemory(&certInfo.NotBefore, &ullTime, sizeof(FILETIME));
    ullTime += SECOND_IN_FILETIME * 60 * 60 * 24 * 365;
    CopyMemory(&certInfo.NotAfter, &ullTime, sizeof(FILETIME));
    certInfo.Issuer.cbData = cSignerName;
    certInfo.Issuer.pbData = pbSignerName;
    certInfo.Subject.cbData = cCertName;
    certInfo.Subject.pbData = pbCertName;
    certInfo.SubjectPublicKeyInfo = *(reinterpret_cast<CERT_PUBLIC_KEY_INFO*>(s_keyDataBuf));

    // create the cert
    DWORD cCertBuf = sizeof(s_certBuf);
    bRet = CryptSignAndEncodeCertificate(hCryptProvSigner,                         // Crypto provider
                                                                   DEFAULT_KEY_SPEC,                   // Key spec, we always use the same
                                                                   DEFAULT_ENCODING,                   // Encoding type, default
                                                                   X509_CERT_TO_BE_SIGNED,             // Structure type - certificate
                                                                   &certInfo,                          // Structure information
                                                                   &certInfo.SignatureAlgorithm,       // Signature algorithm
                                                                   NULL,                               // reserved, must be NULL
                                                                   s_certBuf,                     // hopefully it will fit in 1K
                                                                   &cCertBuf);
    if (!bRet)
        return HRESULT_FROM_WIN32(GetLastError());

    // retrieve the cert context.  pCertContext gets a pointer into the crypto api heap, we must treat it as read only. 
    //  pCertContext must be freed with CertFreeCertificateContext(p); we use a smart pointer to do the free
    pCertContext = (CERT_CONTEXT*)CertCreateCertificateContext(DEFAULT_ENCODING, s_certBuf, cCertBuf);
    if (NULL == pCertContext)
        return HRESULT_FROM_WIN32(GetLastError());
    

    // next attach the private key
    // =================


    // retrieve container name
    hr = GetContainerName(hCryptProvThisCert, &cchContainer, wzContainer);
    if (FAILED(hr))
        return hr;

    // set up key info struct for CAPI call
    keyInfo.pwszContainerName = wzContainer;
    keyInfo.pwszProvName = NULL;
    keyInfo.dwProvType = DEFAULT_PROV_TYPE;
    keyInfo.dwKeySpec = DEFAULT_KEY_SPEC;

     // attach private key
    bRet = CertSetCertificateContextProperty(pCertContext, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyInfo);
    if (!bRet)
        return HRESULT_FROM_WIN32(GetLastError());
            
    // put the cert into the store
    bRet = CertAddCertificateContextToStore(hCertStore, pCertContext, CERT_STORE_ADD_NEW, NULL);
    if (!bRet)
        return HRESULT_FROM_WIN32(GetLastError());

    // make sure the issuer cert is also in the store, if we have an issuer for the cert (ie, the non self signed case)
    if (pIssuerCertContext)
    {
        bRet = CertAddCertificateContextToStore(hCertStore, pIssuerCertContext, CERT_STORE_ADD_NEW, NULL);
        if (!bRet)
            return HRESULT_FROM_WIN32(GetLastError());
    }

    // now export the cert to a file
    hr = WriteStoreToFile(hCertStore, pwzFileName);
    
    return hr;

}

// using cryptoApi, make a cert and export it.  If there is an existing root cert, this will
//  use the existing root cert to sign the local cert.
//  export a local cert to the file <currentdir>\LocalCert.cer
//  export a root cert to the file <currentDir>\RootCert.cer (if one does not already exist)
HRESULT MakeCert(LPCWSTR pwzLocalCertFileName, LPCWSTR pwzLocalCertName, 
                                 LPCWSTR pwzIssuerCertFileName, LPCWSTR pwzIssuerCertName)
{
    HRESULT hr = S_OK;
    XHCERTSTORE hSelfCertStore;
    XHCERTSTORE hSignedCertStore;
    XHCRYPTPROV hCryptProvIssuer;
    XHCRYPTPROV hCryptProvThis;
    XCERT_CONTEXT pIssuerCert;


    // If there is an issuer cert, make sure it exists
    if (pwzIssuerCertFileName)
    {
        hr = ReadCertFromFile(pwzIssuerCertFileName, &pIssuerCert, &hCryptProvIssuer);
        if (FAILED(hr))
            return hr;
    }

    // create this cert key pair (util function from peernet\common)
    hr = CreateCryptProv(NULL, TRUE, &hCryptProvThis);
    if (FAILED(hr))
        return hr;

    // create cert store
    hSelfCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL,
        CERT_STORE_CREATE_NEW_FLAG | CERT_STORE_NO_CRYPT_RELEASE_FLAG, NULL); 
    if (NULL == hSelfCertStore)
        return HRESULT_FROM_WIN32(GetLastError());


    // Make the self signed cert, and save it to a file
    hr = MakeAndExportACert(pwzLocalCertName, pwzLocalCertName, pwzLocalCertFileName, hSelfCertStore, hCryptProvThis, hCryptProvThis, NULL);
        if (FAILED(hr))
            return hr;
        
    // then, sign it if an issuer name was supplied
    //  (surprisingly, the same function does both, since it adds a signing record to existing cert)
    //  FUTURE: this is a bit inefficient, since we write the file twice, we can add a fWrite paramater, and not write the file when it is false
    if (pwzIssuerCertFileName)
    {
        // must create a separate store, or we end up with a single cert and a chain cert in same store, apps can pick wrong cert
        hSignedCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL,
            CERT_STORE_CREATE_NEW_FLAG | CERT_STORE_NO_CRYPT_RELEASE_FLAG, NULL); 
        if (NULL == hSignedCertStore)
            return HRESULT_FROM_WIN32(GetLastError());
        
        hr = MakeAndExportACert(pwzIssuerCertName, pwzLocalCertName, pwzLocalCertFileName, hSignedCertStore, hCryptProvIssuer, hCryptProvThis, pIssuerCert);
    }
    return hr;
}


