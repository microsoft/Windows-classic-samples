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

// Initialize a pkcs10 request from the public key, wrap it in 
// a CMC request, encode the CMC request, submit it to an 
// enterprise CA, and save the response to a file

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollFromPublicKey <Template> <FileOut> ");
    wprintf(L"[<SigningTemplate>]\n");
    wprintf(L"Example: enrollFromPublicKey User Response.out User\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    PCWSTR pwszTemplateName;
    PCWSTR pwszFileOut;
    PCWSTR pwszSigningTemplateName = L"User";    
    CERT_CONTEXT const *pCert = NULL;
    ICertConfig* pCertConfig = NULL;
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IX509CertificateRequestCmc* pCmc = NULL;
    IX509PrivateKey* pKey = NULL;
    IX509PublicKey* pPublicKey = NULL;
    ISignerCertificate* pSignerCertificate = NULL;
    ISignerCertificates* pSignerCertificates = NULL;
    ICertRequest2* pCertRequest2 = NULL;
    BSTR strTemplateName = NULL;
    BSTR strCAConfig = NULL;
    BSTR strRequest = NULL;
    BSTR strRACert = NULL;
    BSTR strDisposition = NULL;
    VARIANT varFullResponse;
    LONG pDisposition = 0;

    // Process command line arguments
    if (argc !=  3 && argc !=  4) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszTemplateName = argv[1];
        pwszFileOut = argv[2];
        if (argc == 4)
            pwszSigningTemplateName = argv[3];
    }

    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;
    
    /* Get the public key */
    
    // Create IX509PrivateKey
    hr = CoCreateInstance(
            __uuidof(CX509PrivateKey),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509PrivateKey),
            (void **) &pKey);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Create the key
    hr = pKey->Create();
    _JumpIfError(hr, error, "Create");

    // Export the public key
    hr = pKey->ExportPublicKey(&pPublicKey);
    _JumpIfError(hr, error, "ExportPublicKey");


    /* Intilialize the CMC request from the public key */

    // Create IX509CertificateRequestPkcs10
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestPkcs10),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestPkcs10),
            (void **) &pPkcs10);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Allocate BSTR for template name
    strTemplateName = SysAllocString(pwszTemplateName);
    if (NULL == strTemplateName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IX509CertificateRequestPkcs10 from public key
    hr = pPkcs10->InitializeFromPublicKey(
            ContextUser,
            pPublicKey,
            strTemplateName);
    _JumpIfError(hr, error, "InitializeFromPublicKey");

    // Create IX509CertificateRequestCmc
    hr = CoCreateInstance(
            _uuidof(CX509CertificateRequestCmc),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            _uuidof(IX509CertificateRequestCmc),
            (void **) &pCmc);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestCmc
    hr = pCmc->InitializeFromInnerRequest(pPkcs10);
    _JumpIfError(hr, error, "InitializeFromInnerRequest");


    /* Sign the CMC request with a signing cert */

    // Find a signing cert
    hr = findCertByKeyUsage(CERT_DIGITAL_SIGNATURE_KEY_USAGE, &pCert);
    if (S_OK != hr) // Cert not found
    {
        // Enroll a signing cert first
        hr = enrollCertByTemplate(pwszSigningTemplateName);
        _JumpIfError(hr, error, "enrollCertByTemplate");    
 
        // Search again
        hr = findCertByKeyUsage(CERT_DIGITAL_SIGNATURE_KEY_USAGE, &pCert);
        _JumpIfError(hr, error, "findCertByKeyUsage");  
    }

    // Verify the certificate chain
    hr = verifyCertContext(pCert, NULL);
    _JumpIfError(hr, error, "verifyCertContext");

    // Convert PCCERT_CONTEXT to BSTR
    strRACert = SysAllocStringByteLen(
            (CHAR const *) pCert->pbCertEncoded, 
            pCert->cbCertEncoded);

    if (NULL == strRACert)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocStringByteLen");
    }

    // Retrieve ISignerCertificates collection from CMC request
    hr = pCmc->get_SignerCertificates(&pSignerCertificates);
    _JumpIfError(hr, error, "get_SignerCertificates");

    // Create ISignerCertificate
    hr = CoCreateInstance(
            __uuidof(CSignerCertificate),  
            NULL,   // pUnkOuter
            CLSCTX_INPROC_SERVER, 
            __uuidof(ISignerCertificate), 
            (void **)&pSignerCertificate); 
   _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize ISignerCertificate from signing cert
    hr = pSignerCertificate->Initialize(
            VARIANT_FALSE,
            VerifyNone,
            XCN_CRYPT_STRING_BINARY,
            strRACert);
   _JumpIfError(hr, error, "Initialize");

    // Add the signing cert into ISignerCertificates collection
    hr = pSignerCertificates->Add(pSignerCertificate);
     _JumpIfError(hr, error, "Add");


    /* Encode the CMC request, submit the request to an enterprise CA */

    // Encode the CMC request
    hr = pCmc->Encode();
    _JumpIfError(hr, error, "Encode");
    
    // Get BSTR of the CMC request
    hr = pCmc->get_RawData(XCN_CRYPT_STRING_BASE64, &strRequest);
    _JumpIfError(hr, error, "Encode");

    // Create ICertConfig
    hr = CoCreateInstance(
            __uuidof(CCertConfig),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(ICertConfig),
            (void**)&pCertConfig);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Get the CA Config from UI
    hr = pCertConfig->GetConfig(CC_UIPICKCONFIG, &strCAConfig);
    _JumpIfError(hr, error, "GetConfig");

    // Initialize ICertRequest2
    hr = CoCreateInstance(
            __uuidof(CCertRequest),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(ICertRequest2),
            (void**)&pCertRequest2);
    _JumpIfError(hr, error, "CoCreateInstance");
  
    // Submit the request
    hr = pCertRequest2->Submit(
            CR_IN_BASE64 | CR_IN_FORMATANY, 
            strRequest, 
            NULL, 
            strCAConfig,
            &pDisposition);   
    _JumpIfError(hr, error, "Submit");

    // Check the submission status
    if (pDisposition != CR_DISP_ISSUED) // Not enrolled
    {
        hr = pCertRequest2->GetDispositionMessage(&strDisposition);
        _JumpIfError(hr, error, "GetDispositionMessage");
        
        if (pDisposition == CR_DISP_UNDER_SUBMISSION) // Pending
        {
            wprintf(L"The submission is pending: %ws\n", strDisposition);
            _JumpError(hr, error, "Submit");
        } 
        else // Failed
        {
            wprintf(L"The submission failed: %ws\n", strDisposition);
            pCertRequest2->GetLastStatus(&hr);
            _JumpError(hr, error, "Submit");
        }
    }

    /* Get the response, and save it to a file */

    // Initialize varFullResponse
    VariantInit(&varFullResponse);
    varFullResponse.vt = VT_BSTR;
    varFullResponse.bstrVal = NULL;

    // Get the full response in binary format
    hr = pCertRequest2->GetFullResponseProperty(
            FR_PROP_FULLRESPONSENOPKCS7,
            0,
            PROPTYPE_BINARY,
            CR_OUT_BINARY,
            &varFullResponse);
    _JumpIfError(hr, error, "GetFullResponseProperty");

    // Save the response to file in base64 format
    hr = EncodeToFileW(
            pwszFileOut, 
            (BYTE const *) varFullResponse.bstrVal, 
            SysStringByteLen(varFullResponse.bstrVal), 
            CR_OUT_BASE64 | DECF_FORCEOVERWRITE);
    _JumpIfError(hr, error, "EncodeToFileW");

error:
    SysFreeString(strTemplateName);
    SysFreeString(strCAConfig);
    SysFreeString(strRequest);
    SysFreeString(strRACert);
    SysFreeString(strDisposition);
    VariantClear(&varFullResponse);
    if (NULL != pKey) pKey->Release();
    if (NULL != pPublicKey) pPublicKey->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pCmc) pCmc->Release();
    if (NULL != pCertRequest2) pCertRequest2->Release();
    if (NULL != pSignerCertificate) pSignerCertificate->Release();
    if (NULL != pSignerCertificates) pSignerCertificates->Release();
    if (NULL != pCert) CertFreeCertificateContext(pCert);
    if (fCoInit) CoUninitialize();
    return hr;
}

