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


// Create and submit custom PKCS10 request for a user; Set 
// Subject Name; Add custom extensions (Enhanced Key Usage, 
// Subject Alternative Name); Submit the request to a standalone 
// CA and use InstallResponse to install it. 

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollCustomPKCS10 <SubjectName> <RFC822Name> <EKU>\n");
    wprintf(L"Example: enrollCustomPKCS10 \"CN=foo,OU=bar,DC=com\" ");
    wprintf(L"User@Domain.com 1.3.6.1.5.5.7.3.2\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    ICertRequest2* pCertRequest2 = NULL;
    ICertConfig* pCertConfig = NULL;
    IX509Enrollment* pEnroll = NULL; 
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IX509Extensions* pExtensions = NULL;
    IX509Extension* pExtension1 = NULL;
    IX509Extension* pExtension2 = NULL;
    IX509ExtensionEnhancedKeyUsage* pExtensionEKU = NULL;
    IX509ExtensionAlternativeNames* pExtensionSAN = NULL;
    IObjectId *pEKUObjectId = NULL;
    IObjectId *pSANObjectId = NULL;
    IObjectIds *pEKUObjectIds = NULL;
    IObjectIds *pSANObjectIds = NULL;
    IAlternativeName * anRfc822Name = NULL;
    IAlternativeNames * pAlternativeNames  = NULL;
    IX500DistinguishedName* pName = NULL;
    PCWSTR pwszSubject;
    PCWSTR pwszRfc822Name;
    PCWSTR pwszEKU;
    BSTR strCAConfig = NULL;
    BSTR strRfc822Name = NULL;
    BSTR strEKU = NULL;
    BSTR strRequest = NULL;
    BSTR strCert = NULL;
    BSTR strDisposition = NULL;
    BSTR strSubject = NULL;
    LONG pDisposition = 0;

    // Process command line arguments
    if (argc !=  4) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszSubject = argv[1];
        pwszRfc822Name = argv[2];
        pwszEKU = argv[3];
    }

    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;

    // Create IX509Enrollment
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestPkcs10),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestPkcs10),
            (void **) &pPkcs10);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509Enrollment
    hr = pPkcs10->Initialize(ContextUser);
    _JumpIfError(hr, error, "Initialize");


    /* Add Subject Name */
    
    // Allocate BSTR for subject name
    strSubject = SysAllocString(pwszSubject);
    if (NULL == strSubject)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IX500DistinguishedName
    hr = CoCreateInstance(
            __uuidof(CX500DistinguishedName),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX500DistinguishedName),
            (void **) &pName);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Encode the subject name
    hr = pName->Encode(strSubject, XCN_CERT_NAME_STR_NONE);
    _JumpIfError(hr, error, "Encode");

    // Add subject name into the pkcs10 request
    hr = pPkcs10->put_Subject(pName);
    _JumpIfError(hr, error, "put_Subject");


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

    // Query interface of IX509Extension
    hr = pExtensionEKU->QueryInterface(
            __uuidof(IX509Extension),
            (VOID **)&pExtension1);
    _JumpIfError(hr, error, "QueryInterface");


    /* Create SAN extention from a RFC822 Name */

    // Create IAlternativeName
    hr = CoCreateInstance(
            __uuidof(CAlternativeName),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IAlternativeName),
            (void **) &anRfc822Name);
    _JumpIfError(hr, error, "CoCreateInstance");


    // Allocate BSTR for RFC822 name
    strRfc822Name = SysAllocString(pwszRfc822Name);
    if (NULL == strRfc822Name)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Initialize IAlternativeName
    hr = anRfc822Name->InitializeFromString(
            XCN_CERT_ALT_NAME_RFC822_NAME, 
            strRfc822Name);                
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
    hr = pAlternativeNames->Add(anRfc822Name);
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

    // Query interface of IX509Extension  
    hr = pExtensionSAN->QueryInterface(
            __uuidof(IX509Extension),
            (VOID **)&pExtension2);
    _JumpIfError(hr, error, "QueryInterface");


    /* Add EKU and SAN extension into CMC request */

    // Get extensions from the CMC request
    hr = pPkcs10->get_X509Extensions(&pExtensions);
    _JumpIfError(hr, error, "get_X509Extensions");

    // Add EKU extension into the request
    hr = pExtensions->Add(pExtension1);
    _JumpIfError(hr, error, "Add");

    // Add SAN extension into the request
    hr = pExtensions->Add(pExtension2);
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
    hr = pEnroll->InitializeFromRequest(pPkcs10);
    _JumpIfError(hr, error, "InitializeFromRequest");

    // Create request
    hr = pEnroll->CreateRequest(
            XCN_CRYPT_STRING_BASE64,
            &strRequest);
    _JumpIfError(hr, error, "CreateRequest");


    /* Submit request to CA, get response and install it */
    
    // Create ICertConfig
    hr = CoCreateInstance(
            __uuidof(CCertConfig),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(ICertConfig),
            (void**)&pCertConfig);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Get CA config from UI
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
    SysFreeString(strRfc822Name);
    SysFreeString(strSubject);
    SysFreeString(strEKU);
    SysFreeString(strRequest);
    SysFreeString(strCert);
    SysFreeString(strDisposition);

    if (NULL != pCertConfig) pCertConfig->Release();
    if (NULL != pCertRequest2) pCertRequest2->Release();
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pExtensions) pExtensions->Release();
    if (NULL != pExtension1) pExtension1->Release();
    if (NULL != pExtension2) pExtension2->Release();
    if (NULL != pExtensionEKU) pExtensionEKU->Release();
    if (NULL != pExtensionSAN) pExtensionSAN->Release();
    if (NULL != pEKUObjectId) pEKUObjectId->Release();
    if (NULL != pSANObjectId) pSANObjectId->Release();
    if (NULL != pEKUObjectIds) pEKUObjectIds->Release();
    if (NULL != pSANObjectIds) pSANObjectIds->Release();
    if (NULL != anRfc822Name) anRfc822Name->Release();
    if (NULL != pAlternativeNames) pAlternativeNames->Release();
    if (NULL != pName) pName->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}


