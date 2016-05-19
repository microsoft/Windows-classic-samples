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


// Simple template based machine enrollment.
// Add friendly name and description.

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"enrollSimpleMachineCert <Template> ");
    wprintf(L"<CertFriendlyName> <CertDescription>\n");
    wprintf(L"Example: enrollSimpleMachineCert ");
    wprintf(L"Machine \"Machine Cert\" \"Simple Machine Cert\"\n");
}


HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])
{
    HRESULT hr = S_OK;
    bool fCoInit = false;
    IX509Enrollment* pEnroll = NULL;
    PCWSTR pwszTemplateName;
    PCWSTR pwszCertFriendlyName;
    PCWSTR pwszCertDescription;
    BSTR strTemplateName = NULL;
    BSTR strCertFriendlyName = NULL;
    BSTR strCertDescription = NULL; 

    // Process command line arguments
    if (argc !=  4) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszTemplateName = argv[1];
        pwszCertFriendlyName = argv[2];
        pwszCertDescription = argv[3];
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

    // Allocate BSTR for certificate friendly name
    strCertFriendlyName = SysAllocString(pwszCertFriendlyName);
    if (NULL == strCertFriendlyName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Allocate BSTR for certificate description
    strCertDescription = SysAllocString(pwszCertDescription);
    if (NULL == strCertDescription)
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

    // Initialize IX509Enrollment 
    hr = pEnroll->InitializeFromTemplateName(
            ContextAdministratorForceMachine,
            strTemplateName);
    _JumpIfError(hr, error, "InitializeFromTemplateName");

    // Add certificate friendly name
    hr = pEnroll->put_CertificateFriendlyName(strCertFriendlyName);
    _JumpIfError(hr, error, "put_CertificateFriendlyName");

    // Add certificate descritpion
    hr = pEnroll->put_CertificateDescription(strCertDescription);
     _JumpIfError(hr, error, "put_CertificateDescription");

    // Enroll
    hr = pEnroll->Enroll();
    _JumpIfError(hr, error, "Enroll"); 

    // Check enrollment status
    hr = checkEnrollStatus(pEnroll);
    _JumpIfError(hr, error, "checkEnrollStatus"); 

error:
    SysFreeString(strTemplateName);
    SysFreeString(strCertFriendlyName);
    SysFreeString(strCertDescription);
    if (NULL != pEnroll) pEnroll->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}
