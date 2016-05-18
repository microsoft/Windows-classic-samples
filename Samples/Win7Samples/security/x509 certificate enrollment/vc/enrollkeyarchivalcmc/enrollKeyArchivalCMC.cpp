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


// Create a CMC key archival request based on a template, submit 
// the request to an enterprise CA and install the response

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollKeyArchivalCMC <Template> \n");
    wprintf(L"Example: enrollKeyArchivalCMC KeyArchival\n");
    }

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    ICertRequest2* pCertRequest2 = NULL;
    ICertConfig* pCertConfig = NULL;
    IX509Enrollment* pEnroll = NULL; 
    IX509CertificateRequest* pRequest = NULL;
    IX509CertificateRequestCmc* pCmc = NULL;
    PCWSTR pwszTemplateName;
    BSTR strTemplateName = NULL;
    BSTR strCAConfig = NULL;
    BSTR strCertXchg = NULL;
    BSTR strRequest = NULL;
    BSTR strDisposition = NULL;
    LONG pDisposition = 0;
    VARIANT varFullResponse;
    
    // Process command line arguments
    if (argc !=  2) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszTemplateName = argv[1];
    }
    
    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;

    // Allocate BSTR for template name
    strTemplateName = SysAllocString(pwszTemplateName);
    if (NULL == strTemplateName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IX509CertificateRequestCmc
    hr = CoCreateInstance(
            _uuidof(CX509CertificateRequestCmc),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            _uuidof(IX509CertificateRequestCmc),
            (void **) &pCmc);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestCmc
    hr = pCmc->InitializeFromTemplateName(
            ContextUser,       
            strTemplateName);   
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Set archiving the private key 
    hr = pCmc->put_ArchivePrivateKey(VARIANT_TRUE);
    _JumpIfError(hr, error, "put_ArchivePrivateKey");

    // Create ICertRequest2
    hr = CoCreateInstance(
            __uuidof(CCertRequest),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(ICertRequest2),
            (void**)&pCertRequest2);
    _JumpIfError(hr, error, "CoCreateInstance");

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

    // Get the CA Exchange certificate
    hr = pCertRequest2->GetCACertificate(
            TRUE,
            strCAConfig,
            CR_OUT_BINARY,
            &strCertXchg);
    _JumpIfError(hr, error, "GetCACertificate");

    // Set CMC request's CA exchange certificate
    hr = pCmc->put_KeyArchivalCertificate(
            XCN_CRYPT_STRING_BINARY, 
            strCertXchg);
    _JumpIfError(hr, error, "put_KeyArchivalCertificate");

    // Create IX509Enrollment
    hr = CoCreateInstance(
            __uuidof(CX509Enrollment),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509Enrollment),
            (void **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509Enrollment from CMC request
    hr = pEnroll->InitializeFromRequest(pCmc);
    _JumpIfError(hr, error, "InitializeFromRequest");

    // Create request
    hr = pEnroll->CreateRequest(
            XCN_CRYPT_STRING_BASE64,
            &strRequest);
    _JumpIfError(hr, error, "CreateRequest");

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

    // Initialize varFullResponse
    VariantInit(&varFullResponse);
    varFullResponse.vt = VT_BSTR;
    varFullResponse.bstrVal = NULL;

    // Get the full response
    hr = pCertRequest2->GetFullResponseProperty(
            FR_PROP_FULLRESPONSENOPKCS7,
            0,
            PROPTYPE_BINARY,
            CR_OUT_BASE64,
            &varFullResponse);
    _JumpIfError(hr, error, "GetFullResponseProperty");

    // Install the response
    hr = pEnroll->InstallResponse(
            AllowNone, 
            varFullResponse.bstrVal, 
            XCN_CRYPT_STRING_BASE64, 
            NULL);
    _JumpIfError(hr, error, "InstallResponse");

error:
    SysFreeString(strTemplateName);
    SysFreeString(strCAConfig);
    SysFreeString(strCertXchg);
    SysFreeString(strRequest);
    SysFreeString(strDisposition);
    VariantClear(&varFullResponse);
    if (NULL != pCertRequest2) pCertRequest2->Release();
    if (NULL != pCertConfig) pCertConfig->Release();
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pRequest) pRequest->Release();
    if (NULL != pCmc) pCmc->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}

