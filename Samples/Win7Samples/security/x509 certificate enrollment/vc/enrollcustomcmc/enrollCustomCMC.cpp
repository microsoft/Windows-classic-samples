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


// Create and submit custom CMC request for a machine. 
// Add custom extensions (Enhanced Key Usage, Name value 
// pair) and Subject Alternative name, submit the request
// to a standalone CA and use InstallResponse to install it. 

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollCustomCMC <Name> <Value> <DNS> <EKU> \n");
    wprintf(L"Example: enrollCustomCMC Name Value ");
    wprintf(L"www.adatum.com 1.3.6.1.5.5.7.3.1\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{
    HRESULT hr = S_OK;
    bool fCoInit = false;
    ICertRequest2* pCertRequest2 = NULL;
    ICertConfig* pCertConfig = NULL;
    IX509Enrollment* pEnroll = NULL; 
    IX509CertificateRequest* pRequest = NULL;
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IX509CertificateRequestCmc* pCmc = NULL;
    IX509Extensions* pExtensions = NULL;
    IX509Extension* pExtension1 = NULL;
    IX509Extension* pExtension2 = NULL;
    IX509ExtensionEnhancedKeyUsage* pExtensionEKU = NULL;
    IX509ExtensionAlternativeNames* pExtensionSAN = NULL;
    IObjectId *pEKUObjectId = NULL;
    IObjectId *pSANObjectId = NULL;
    IObjectIds *pEKUObjectIds = NULL;
    IObjectIds *pSANObjectIds = NULL;
    IAlternativeName * anDnsName = NULL;
    IAlternativeNames * pAlternativeNames  = NULL;
    IX509NameValuePair *pPair = NULL;
    IX509NameValuePairs *pPairs = NULL;
    PCWSTR pwszName;
    PCWSTR pwszValue;
    PCWSTR pwszDnsName;
    PCWSTR pwszEKU;
    BSTR strCAConfig = NULL;
    BSTR strDnsName = NULL;
    BSTR strName = NULL;
    BSTR strValue = NULL;
    BSTR strEKU = NULL;
    BSTR strRequest = NULL;
    BSTR strCert = NULL;
    BSTR strDisposition = NULL;
    LONG pDisposition = 0;

    // Process command line arguments
    if (argc !=  5) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszName = argv[1];
        pwszValue = argv[2];
        pwszDnsName = argv[3];
        pwszEKU = argv[4];
    }

    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;

    // Create IX509CertificateRequestPkcs10
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestPkcs10),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestPkcs10),
            (void **) &pPkcs10);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestPkcs10
    hr = pPkcs10->Initialize(ContextMachine);
    _JumpIfError(hr, error, "Initialize");

    // Create IX509CertificateRequestCmc
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestCmc),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestCmc),
            (void **) &pCmc);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize CMC request
    hr = pCmc->InitializeFromInnerRequest(pPkcs10);
    _JumpIfError(hr, error, "InitializeFromInnerRequest");

    
    /* Create EKU extention from an OID */
    
    // Create IObjectId
    hr = CoCreateInstance(
            __uuidof(CObjectId),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IObjectId),
            (void **) &pEKUObjectId);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Allocate BSTR for EKU OID
    strEKU = SysAllocString(pwszEKU);
    if (NULL == strEKU)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IObjectId from EKU OID
    hr = pEKUObjectId->InitializeFromValue(strEKU);
    _JumpIfError(hr, error, "InitializeFromValue");

    // Create IObjectIds
    hr = CoCreateInstance(
            __uuidof(CObjectIds),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IObjectIds),
            (void **) &pEKUObjectIds);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Add IObjectId into IObjectIds collection
    hr = pEKUObjectIds->Add(pEKUObjectId);
    _JumpIfError(hr, error, "Add");

    // Create IX509ExtensionEnhancedKeyUsage
    hr = CoCreateInstance(
            __uuidof(CX509ExtensionEnhancedKeyUsage),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509ExtensionEnhancedKeyUsage),
            (void **) &pExtensionEKU);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509ExtensionEnhancedKeyUsage from IObjectIds
    hr = pExtensionEKU->InitializeEncode(pEKUObjectIds);
    _JumpIfError(hr, error, "InitializeEncode");

    // QueryInterface of IX509Extension
    hr = pExtensionEKU->QueryInterface(
            __uuidof(IX509Extension),
            (VOID **)&pExtension1);
    _JumpIfError(hr, error, "QueryInterface");


    /* Create SAN extention from a DNS Name */

    // Create IAlternativeName
    hr = CoCreateInstance(
            __uuidof(CAlternativeName),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IAlternativeName),
            (void **) &anDnsName);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Allocate BSTR for DNS name
    strDnsName = SysAllocString(pwszDnsName);
    if (NULL == strDnsName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IAlternativeName
    hr = anDnsName->InitializeFromString(
            XCN_CERT_ALT_NAME_DNS_NAME,     
            strDnsName);              
    _JumpIfError(hr, error, "InitializeFromString");

    // Create IAlternativeNames
    hr = CoCreateInstance(
            __uuidof(CAlternativeNames),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IAlternativeNames),
            (void **) &pAlternativeNames);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Add IAlternativeName into IAlternativeNames collection
    hr = pAlternativeNames->Add(anDnsName);
    _JumpIfError(hr, error, "Add");

    // Create IX509ExtensionAlternativeNames
    hr = CoCreateInstance(
            __uuidof(CX509ExtensionAlternativeNames),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509ExtensionAlternativeNames),
            (void **) &pExtensionSAN);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509ExtensionAlternativeNames
    hr = pExtensionSAN->InitializeEncode(pAlternativeNames);
    _JumpIfError(hr, error, "InitializeEncode");

    // QueryInterface of IX509Extension  
    hr = pExtensionSAN->QueryInterface(
            __uuidof(IX509Extension),
            (VOID **)&pExtension2);
    _JumpIfError(hr, error, "QueryInterface");


    /* Add EKU and SAN extension into CMC request */

    // Get extensions from the CMC request
    hr = pCmc->get_X509Extensions(&pExtensions);
    _JumpIfError(hr, error, "get_X509Extensions");

    // Add EKU extension into the request
    hr = pExtensions->Add(pExtension1);
    _JumpIfError(hr, error, "Add");

    // Add SAN extension into the request
    hr = pExtensions->Add(pExtension2);
    _JumpIfError(hr, error, "Add");


    /* Create name value pair and add it into CMC request */

    // Create IX509NameValuePair
    hr = CoCreateInstance(
            __uuidof(CX509NameValuePair),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509NameValuePair),
            (void **) &pPair);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Allocate BSTR for name
    strName = SysAllocString(pwszName);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Allocate BSTR for value
    strValue = SysAllocString(pwszValue);
    if (NULL == strValue)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IX509NameValuePair
    hr = pPair->Initialize(strName, strValue);
    _JumpIfError(hr, error, "Initialize");

    // Get IX509NameValuePairs from CMC request
    hr = pCmc->get_NameValuePairs(&pPairs);
    _JumpIfError(hr, error, "get_NameValuePairs");

    // Add IX509NameValuePair into IX509NameValuePairs collection
    hr = pPairs->Add(pPair);
    _JumpIfError(hr, error, "Add");


    /* Create enrollment request */

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


    /* Submit request to SA CA, get response and install it */
    
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

    // Create ICertRequest2
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

    // Get the certifcate
    hr = pCertRequest2->GetCertificate(
            CR_OUT_BASE64 | CR_OUT_CHAIN, 
            &strCert);
    _JumpIfError(hr, error, "GetCertificate");

    // Install the certifcate
    hr = pEnroll->InstallResponse(
            AllowNone, 
            strCert, 
            XCN_CRYPT_STRING_BASE64, 
            NULL);
    _JumpIfError(hr, error, "InstallResponse");

error:
    SysFreeString(strCAConfig);
    SysFreeString(strDnsName);
    SysFreeString(strName);
    SysFreeString(strValue);
    SysFreeString(strEKU);
    SysFreeString(strRequest);
    SysFreeString(strCert);
    SysFreeString(strDisposition);

    if (NULL != pCertRequest2) pCertRequest2->Release();
    if (NULL != pCertConfig) pCertConfig->Release();
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pRequest) pRequest->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pCmc) pCmc->Release();
    if (NULL != pExtensions) pExtensions->Release();
    if (NULL != pExtension1) pExtension1->Release();
    if (NULL != pExtension2) pExtension2->Release();
    if (NULL != pExtensionEKU) pExtensionEKU->Release();
    if (NULL != pExtensionSAN) pExtensionSAN->Release();
    if (NULL != pEKUObjectId) pEKUObjectId->Release();
    if (NULL != pSANObjectId) pSANObjectId->Release();
    if (NULL != pEKUObjectIds) pEKUObjectIds->Release();
    if (NULL != pSANObjectIds) pSANObjectIds->Release();
    if (NULL != anDnsName) anDnsName->Release();
    if (NULL != pAlternativeNames) pAlternativeNames->Release();
    if (NULL != pPair) pPair->Release();
    if (NULL != pPairs) pPairs->Release(); 
    if (fCoInit) CoUninitialize();
    return hr;
}
