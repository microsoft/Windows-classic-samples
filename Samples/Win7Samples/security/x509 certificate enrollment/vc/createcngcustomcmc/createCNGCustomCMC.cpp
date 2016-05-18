//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------


// Create a CNG key with custom options (set CNG provider, asymetric 
// algorithm, export policy, key protection), create custom CMC request 
// based on the CNG key, set hash algorithm and alternative signature
// format. Encode the CMC request and save it to a file. 

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"createCNGCustomCMC <ProviderName> <AlgName> <HashAlgName> ");
    wprintf(L"<FileOut> [AlternateSignature]\n");
    wprintf(L"Example: createCNGCustomCMC ");
    wprintf(L"\"Microsoft Software Key Storage Provider\" ");
    wprintf(L"ECDSA_P521 MD5 Cmc.out AlternateSignature\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    PCWSTR pwszProvName;
    PCWSTR pwszAlgName;
    PCWSTR pwszHashAlgName;
    PCWSTR pwszFileOut;
    VARIANT_BOOL fAlternateSignature;
    X509PrivateKeyExportFlags ExportPolicy = XCN_NCRYPT_ALLOW_EXPORT_NONE;
    X509PrivateKeyProtection KeyProtection = XCN_NCRYPT_UI_NO_PROTECTION_FLAG;  
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IX509CertificateRequestCmc* pCmc = NULL;
    IX509PrivateKey* pKey = NULL;
    IObjectId* pAlg = NULL;
    IObjectId* pHashAlg = NULL;
    BSTR strProvName = NULL;
    BSTR strAlgName = NULL;
    BSTR strHashAlgName = NULL;
    BSTR strRequest = NULL;

    // Process command line arguments
    if (argc != 5 && argc != 6) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszProvName = argv[1];
        pwszAlgName = argv[2];
        pwszHashAlgName = argv[3];
        pwszFileOut = argv[4];
        if (argc == 6 && 0 == _wcsicmp(argv[5], L"AlternateSignature"))
            fAlternateSignature = VARIANT_TRUE;
        else 
            fAlternateSignature = VARIANT_FALSE;
    }

    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;

    // Create IX509PrivateKey
    hr = CoCreateInstance(
            __uuidof(CX509PrivateKey),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509PrivateKey),
            (void **) &pKey);
    _JumpIfError(hr, error, "CoCreateInstance");
    
    // The provider is a CNG CSP
    hr = pKey->put_LegacyCsp(VARIANT_FALSE);
    _JumpIfError(hr, error, "put_LegacyCsp");

    // Allocate BSTR for provider name
    strProvName = SysAllocString(pwszProvName);
    if (NULL == strProvName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Set provider name for private key
    hr = pKey->put_ProviderName(strProvName);
    _JumpIfError(hr, error, "put_ProviderName");

    // Allocate BSTR for algorithm name
    strAlgName = SysAllocString(pwszAlgName);
    if (NULL == strAlgName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IObjectId
    hr = CoCreateInstance(
            __uuidof(CObjectId),
            NULL,     // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IObjectId),
            (void **) &pAlg);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IObjectId from strAlgName
    hr = pAlg->InitializeFromAlgorithmName(
            XCN_CRYPT_PUBKEY_ALG_OID_GROUP_ID,
            XCN_CRYPT_OID_INFO_PUBKEY_ANY,
            AlgorithmFlagsNone,
            strAlgName);
    _JumpIfError(hr, error, "InitializeFromAlgorithmName");

    // Set algorithm for private key
    hr = pKey->put_Algorithm(pAlg);
    _JumpIfError(hr, error, "put_Algorithm");
 
    // Set key proection for private key
    hr = pKey->put_KeyProtection(KeyProtection);
    _JumpIfError(hr, error, "put_KeyProtection");

    // Set export policy for private key
    hr = pKey->put_ExportPolicy(ExportPolicy);
    _JumpIfError(hr, error, "put_ExportPolicy");

    // Create the key
    hr = pKey->Create();
    _JumpIfError(hr, error, "Create");

    // Create IX509CertificateRequestPkcs10
    hr = CoCreateInstance(
            _uuidof(CX509CertificateRequestPkcs10),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            _uuidof(IX509CertificateRequestPkcs10),
            (void **) &pPkcs10);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestPkcs10 from private key
    hr = pPkcs10->InitializeFromPrivateKey(
            ContextUser,
            pKey,
            NULL);
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Create IX509CertificateRequestCmc
    hr = CoCreateInstance(
            _uuidof(CX509CertificateRequestCmc),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            _uuidof(IX509CertificateRequestCmc),
            (void **) &pCmc);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestCmc from inner pkcs10 request
    hr = pCmc->InitializeFromInnerRequest(pPkcs10);
    _JumpIfError(hr, error, "InitializeFromInnerRequest");

    // Set AlternateSignature for CMC request
    hr = pCmc->put_AlternateSignatureAlgorithm(fAlternateSignature);
    _JumpIfError(hr, error, "put_AlternateSignatureAlgorithm");

    // Allocate BSTR for hash algorithm name
    strHashAlgName = SysAllocString(pwszHashAlgName);
    if (NULL == strHashAlgName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IObjectId
    hr = CoCreateInstance(
            __uuidof(CObjectId),
            NULL,     // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IObjectId),
            (void **) &pHashAlg);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IObjectId from strHashAlgName
    hr = pHashAlg->InitializeFromAlgorithmName(
            XCN_CRYPT_HASH_ALG_OID_GROUP_ID,
            XCN_CRYPT_OID_INFO_PUBKEY_ANY,
            AlgorithmFlagsNone,
            strHashAlgName);
    _JumpIfError(hr, error, "InitializeFromAlgorithmName");

    // Set hash algorithm for CMC request
    hr = pCmc->put_HashAlgorithm(pHashAlg);
    _JumpIfError(hr, error, "put_HashAlgorithm");


    // Encode the CMC request
    hr = pCmc->Encode();
    _JumpIfError(hr, error, "Encode");
    
    // Get BSTR of the CMC request
    hr = pCmc->get_RawData(XCN_CRYPT_STRING_BINARY, &strRequest);
    _JumpIfError(hr, error, "Encode");
    
    // Save request to file in base64 format
    hr = EncodeToFileW(pwszFileOut, 
            (BYTE const *) strRequest, 
            SysStringByteLen(strRequest), 
            CR_OUT_BASE64 | DECF_FORCEOVERWRITE);
    _JumpIfError(hr, error, "EncodeToFileW");

error:
    SysFreeString(strProvName);
    SysFreeString(strAlgName);
    SysFreeString(strHashAlgName);
    SysFreeString(strRequest);
    if (NULL != pKey) pKey->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pCmc) pCmc->Release();
    if (NULL != pAlg) pAlg->Release();
    if (NULL != pHashAlg) pHashAlg->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}
