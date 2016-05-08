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


// Create a non-renewal pkcs7 request with InheritPrivateKey and 
// InheritTempalte flags, enroll to an enterprise CA

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollPKCS7 <Template>\n");
    wprintf(L"Example: enrollPKCS7 User\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{

    HRESULT hr = S_OK;
    bool fCoInit = false;
    IX509Enrollment* pEnroll = NULL;
    IX509CertificateRequestPkcs7* pPkcs7 = NULL;
    ISignerCertificate* pSignerCertificate = NULL;
    CERT_CONTEXT const *pCert = NULL;
    PCWSTR pwszTemplateName;
    BSTR strOldCert = NULL;
    
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


    
    /* Find a certificate first */

    // Find a certificate by template name
    hr = findCertByTemplate(pwszTemplateName, &pCert);
    if (S_OK != hr) // Cert not found
    {
        // Enroll a cert of the template name first
        hr = enrollCertByTemplate(pwszTemplateName);
        _JumpIfError(hr, error, "enrollCertByTemplate");    
 
        // Search again
        hr = findCertByTemplate(pwszTemplateName, &pCert);
        _JumpIfError(hr, error, "findCertByTemplate");  
    }

    // Verify the certificate chain
    hr = verifyCertContext(pCert, NULL);
    _JumpIfError(hr, error, "verifyCertContext");

    // Convert PCCERT_CONTEXT to BSTR
    strOldCert = SysAllocStringByteLen(
            (CHAR const *) pCert->pbCertEncoded, 
            pCert->cbCertEncoded);

    if (NULL == strOldCert)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocStringByteLen");
    }

    /* Create a pkcs7 request and sign it by the signer cert */

    // Create IX509CertificateRequestPkcs7
    hr = CoCreateInstance(
            __uuidof(CX509CertificateRequestPkcs7),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509CertificateRequestPkcs7),
            (void **) &pPkcs7);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509CertificateRequestPkcs7 from old cert
    hr = pPkcs7->InitializeFromCertificate(
        ContextUser,                          
        VARIANT_FALSE,                        
        strOldCert,                           
        XCN_CRYPT_STRING_BINARY,              
        (X509RequestInheritOptions)(InheritPrivateKey|InheritTemplateFlag));
    _JumpIfError(hr, error, "InitializeFromCertificate");
    
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
            strOldCert);
   _JumpIfError(hr, error, "Initialize");

    // Add signing cert into Pkcs7 request
    hr = pPkcs7->put_SignerCertificate(pSignerCertificate);
    _JumpIfError(hr, error, "put_SignerCertificates");

    // Create IX509Enrollment
    hr = CoCreateInstance(
            __uuidof(CX509Enrollment),
            NULL,       // pUnkOuter
            CLSCTX_INPROC_SERVER,
            __uuidof(IX509Enrollment),
            (void **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Initialize IX509Enrollment
    hr = pEnroll->InitializeFromRequest(pPkcs7);
    _JumpIfError(hr, error, "InitializeFromRequest");

    // Enroll
    hr = pEnroll->Enroll();
    _JumpIfError(hr, error, "Enroll"); 

    // Check enrollment status
    hr = checkEnrollStatus(pEnroll);
    _JumpIfError(hr, error, "checkEnrollStatus"); 

error:
    SysFreeString(strOldCert);
    if (NULL != pEnroll) pEnroll->Release();
    if (NULL != pPkcs7) pPkcs7->Release();
    if (NULL != pSignerCertificate) pSignerCertificate->Release();
    if (NULL != pCert) CertFreeCertificateContext(pCert);
    if (fCoInit) CoUninitialize();
    return hr;
}

