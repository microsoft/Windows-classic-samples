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


// InstallResponse for a PFX file  

#include <stdio.h>
#include <certenroll.h>
#include <certsrv.h>
#include <certcli.h>
#include <wincrypt.h>
#include "enrollCommon.h"

void Usage()
{
    wprintf(L"Usage:\n");
    wprintf(L"installResponseFromPFX <FileIn> <Password>\n");
    wprintf(L"Example: installResponseFromPFX cert.pfx 1111\n");
}

HRESULT __cdecl wmain(__in int argc, __in_ecount(argc) wchar_t *argv[])	
{
    HRESULT hr = S_OK;
    bool fCoInit = false;
    PCWSTR pwszFileIn;
    PCWSTR pwszPassword;
    IX509Enrollment *pEnroll = NULL;
    BSTR strPassword = NULL;
    BSTR strResponse = NULL;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    
    // Process command line arguments
    if (argc !=  3) {
        Usage();
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid arg");
    }
    else
    {
        pwszFileIn = argv[1];
        pwszPassword = argv[2];
    }

    // CoInitializeEx
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _JumpIfError(hr, error, "CoInitializeEx");
    fCoInit = true;

    // Allocate BSTR for password
    strPassword = SysAllocString(pwszPassword);
    if (NULL == strPassword)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    // Read the PFX file, try base64 format first
    hr = DecodeFileW(pwszFileIn, &pbReq, &cbReq, CRYPT_STRING_BASE64_ANY);
    if (S_OK != hr) // If fails, try binary format
    {
        hr = DecodeFileW(pwszFileIn, &pbReq, &cbReq, CRYPT_STRING_BINARY);
        _JumpIfError(hr, error, "DecodeFileW");
    }

    // Convert WCHAR * to BSTR
    if (!convertWszToBstr(&strResponse, (WCHAR const *) pbReq, cbReq))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "convertWszToBstr");
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
    hr =pEnroll->Initialize(ContextUser);
    _JumpIfError(hr, error, "CoCreateInstance");

    // Install the PFX file
    hr =pEnroll->InstallResponse(
            AllowNone, 
            strResponse, 
            XCN_CRYPT_STRING_ANY, 
            strPassword);
    _JumpIfError(hr, error, "InstallResponse");

error:
    SysFreeString(strPassword);
    SysFreeString(strResponse);
    if (NULL != pbReq) LocalFree(pbReq);
    if (NULL != pEnroll) pEnroll->Release();
    if (fCoInit) CoUninitialize();
    return hr;
}

