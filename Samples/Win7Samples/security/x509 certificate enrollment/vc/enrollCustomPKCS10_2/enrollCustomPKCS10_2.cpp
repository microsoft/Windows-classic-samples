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

// Create and submit custom PKCS10 request for a user; Create a
// collection of ICspInformations for the pkcs10 class and private
// key class to use; Enroll to an enterprise CA.

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollCustomPKCS10_2 <Template> <ProviderName>\n");
    wprintf(L"Example: enrollCustomPKCS10_2 User ");
    wprintf(L"\"Microsoft Enhanced Cryptographic Provider v1.0\"\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    IX509Enrollment* pEnroll = NULL; 
    IX509CertificateRequest* pRequest = NULL;
    IX509CertificateRequest* pInnerRequest = NULL;
    IX509CertificateRequestPkcs10* pPkcs10 = NULL;
    IX509PrivateKey* pPrivateKey = NULL;
    ICspInformations* pCspInfos = NULL;
    ICspStatus* pCspStatus = NULL;
    PCWSTR pwszTemplateName;
    PCWSTR pwszProvName;
    BSTR strTemplateName = NULL;
    BSTR strProvName = NULL;
    
    // Process command line arguments
    if (argc !=  3) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszTemplateName = argv[1];
        pwszProvName = argv[2];
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

    // Allocate BSTR for provider name
    strProvName = SysAllocString(pwszProvName);
    if (NULL == strProvName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Create IX509Enrollment
    hr = CoCreateInstance(
            __uuidof(CX509Enrollment),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509Enrollment),
            (void **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initiate IX509Enrollment
    hr = pEnroll->InitializeFromTemplateName(
            ContextUser,     
            strTemplateName);
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Retrieve the request
    hr = pEnroll->get_Request(&pRequest);
    _JumpIfError(hr, error, "get_Request");
  
    // Get the innermost request
    hr = pRequest->GetInnerRequest(LevelInnermost, &pInnerRequest);
     _JumpIfError(hr, error, "GetInnerRequest");

    // QueryInterface for the pkcs10 request 
    hr = pInnerRequest->QueryInterface(
            __uuidof(IX509CertificateRequestPkcs10),
            (VOID **)&pPkcs10);
    _JumpIfError(hr, error, "QueryInterface");
    
    // Retrieve the private key
    hr = pPkcs10->get_PrivateKey(&pPrivateKey);
    _JumpIfError(hr, error, "get_PrivateKey");

    // Create ICspInformations
    hr = CoCreateInstance(
            __uuidof(CCspInformations), 
            NULL,   // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(ICspInformations),
            (void **)&pCspInfos);  
     _JumpIfError(hr, error, "CoCreateInstance");

    // Add available csps
    hr = pCspInfos->AddAvailableCsps();
    _JumpIfError(hr, error, "AddAvailableCsps");

    // Get the ICspStatus
    hr = pCspInfos->GetCspStatusFromProviderName(
            strProvName,
            XCN_AT_KEYEXCHANGE,
            &pCspStatus);
    _JumpIfError(hr, error, "GetCspStatusFromProviderName");

    // Set CspStatus to the private key
    hr = pPrivateKey->put_CspStatus(pCspStatus);
     _JumpIfError(hr, error, "put_CspStatus");

    // Enroll
    hr = pEnroll->Enroll();
    _JumpIfError(hr, error, "Enroll"); 

    // Check enrollment status
    hr = checkEnrollStatus(pEnroll);
    _JumpIfError(hr, error, "checkEnrollStatus"); 

error:
    SysFreeString(strTemplateName);
    SysFreeString(strProvName);
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pRequest) pRequest->Release();
    if (NULL != pInnerRequest) pInnerRequest->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pPrivateKey) pPrivateKey->Release();
    if (NULL != pCspInfos) pCspInfos->Release();
    if (NULL != pCspStatus) pCspStatus->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}
