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


// Simple template based user enrollment.
// Add Subject name and set key size.

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollSimpleUserCert <Template> <SubjectName> <KeyLength>\n");
    wprintf(L"Example: enrollSimpleUserCert ");
    wprintf(L"User \"CN=foo,OU=bar,DC=com\" 1024\n");
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
    IX500DistinguishedName* pName = NULL;
    PCWSTR pwszTemplateName;
    PCWSTR pwszSubject;
    LONG lKeyLen;
    BSTR strTemplateName = NULL;
    BSTR strSubject = NULL;

    // Process command line arguments
    if (argc !=  4) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszTemplateName = argv[1];
        pwszSubject = argv[2];
        lKeyLen = _wtol(argv[3]);
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

    // Allocate BSTR for subject name
    strSubject = SysAllocString(pwszSubject);
    if (NULL == strSubject)
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
    
    // Set the key length
    hr = pPrivateKey->put_Length(lKeyLen);
    _JumpIfError(hr, error, "put_Length");

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

    // Enroll
    hr = pEnroll->Enroll();
    _JumpIfError(hr, error, "Enroll"); 

    // Check enrollment status
    hr = checkEnrollStatus(pEnroll);
    _JumpIfError(hr, error, "checkEnrollStatus"); 

error:
    SysFreeString(strTemplateName);
    SysFreeString(strSubject);
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pRequest) pRequest->Release();
    if (NULL != pInnerRequest) pInnerRequest->Release();
    if (NULL != pPkcs10) pPkcs10->Release();
    if (NULL != pPrivateKey) pPrivateKey->Release();
    if (NULL != pName) pName->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}
