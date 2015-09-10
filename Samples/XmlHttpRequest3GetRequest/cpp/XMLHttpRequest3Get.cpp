//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//     This sample demonstrates how to use IXMLHttpRequest3 interface to
//     asynchronously sending HTTP GET requests and receiving HTTP responses.
//
//     XMLHttpRequest3Get <Url>
//
//     where:
//
//     Url        is the url for the HTTP request.
//

#include <SDKDDKVer.h>
#include <tchar.h>
#include <msxml6.h>
#include <wrl.h>
#include "XMLHttpRequest3Callback.h"

using namespace Microsoft::WRL;

VOID
Get(
    _In_ PCWSTR pcwszUrl,
    _Inout_ BOOL *pfRetry,
    _Inout_ DWORD *pdwCertIgnoreFlags,
    _Inout_ DWORD *pcIssuerList,
    _Inout_ const WCHAR ***prgpwszIssuerList
)
{
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;
    BOOL fAbort = TRUE;
    ComPtr<IXMLHTTPRequest3> spXHR;
    ComPtr<CXMLHttpRequest3Callback> spXhrCallback;
    BOOL fRetry = *pfRetry;
    DWORD dwCertIgnoreFlags = *pdwCertIgnoreFlags;
    DWORD cIssuerList = *pcIssuerList;
    const WCHAR **rgpwszIssuerList = *prgpwszIssuerList;
    BYTE ClientCertHash[20] = {};
    DWORD cbClientCertHash = sizeof ClientCertHash;

    //
    // Create an object of the IID_IXMLHTTPRequest3 class.
    //

    hr = CoCreateInstance(CLSID_FreeThreadedXMLHTTP60,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&spXHR));
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Create an object of the CXMLHttpRequest3Callback class and initialize it.
    //

    hr = MakeAndInitialize<CXMLHttpRequest3Callback>(&spXhrCallback);
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Send a HTTP GET request.
    //

    hr = spXHR->Open(L"GET",              // Method.
                     pcwszUrl,            // Url.
                     spXhrCallback.Get(), // Callback.
                     NULL,                // Username.
                     NULL,                // Password.
                     NULL,                // Proxy username.
                     NULL);               // Proxy password.
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // On retry, set cert ignore flags.
    //

    if (fRetry && dwCertIgnoreFlags != 0)
    {
        hr = spXHR->SetProperty(XHR_PROP_IGNORE_CERT_ERRORS, dwCertIgnoreFlags);
    }

    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // On retry, set client cert.
    //

    if (fRetry && cIssuerList != 0 && rgpwszIssuerList != NULL)
    {
        hr = spXhrCallback->SelectCert(cIssuerList,
                                       rgpwszIssuerList,
                                       &cbClientCertHash,
                                       ClientCertHash);
        cIssuerList = 0;
        rgpwszIssuerList = NULL;

        if (FAILED(hr))
        {
            goto Exit;
        }

        if (cbClientCertHash > sizeof ClientCertHash)
        {
            hr = E_UNEXPECTED;
            goto Exit;
        }

        hr = spXHR->SetClientCertificate(cbClientCertHash, ClientCertHash, NULL);

        if (FAILED(hr))
        {
            goto Exit;
        }
    }

    //
    // Send the request to the server.
    //

    hr = spXHR->Send(NULL, 0);
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Waiting for the completion of the request.
    // Callers needing to receive completion or status events on a STA or UI
    // thread must use a mechanism that will not block the threads window message
    // pump. One example is by posting a window message to the STA or UI thread
    // window handle.
    //

    hr = spXhrCallback->WaitForComplete(&dwStatus);

    if (FAILED(hr))
    {
        goto Exit;
    }

    fAbort = FALSE;

Exit:

    //
    // Save certificate-related values for retry if applicable.
    //

    if (!fRetry && SUCCEEDED(spXhrCallback->GetCertResult(&fRetry,
                                                          &dwCertIgnoreFlags,
                                                          &cIssuerList,
                                                          &rgpwszIssuerList)))
    {
        *pfRetry = fRetry;
        *pdwCertIgnoreFlags = dwCertIgnoreFlags;
        *pcIssuerList = cIssuerList;
        *prgpwszIssuerList = rgpwszIssuerList;
    }

    if (FAILED(hr))
    {
        wprintf(L"Failed, Error code = 0x%08x.\n", hr);
    }
    else
    {
        wprintf(L"Succeed, Status code = %u.\n", dwStatus);
    }

    if (fAbort)
    {
        spXHR->Abort();
    }
}

int
__cdecl
wmain(
    int argc,
    _In_reads_(argc) WCHAR **argv
)
{
    HRESULT hr = S_OK;
    BOOL fCoInit = FALSE;
    BOOL fRetry = FALSE;
    DWORD dwCertIgnoreFlags = 0;
    DWORD cIssuerList = 0;
    const WCHAR **rgpwszIssuerList = NULL;

    if (argc != 2)
    {
        wprintf(L"Usage: %s <Url>\n", argv[0]);
        goto Exit;
    }

    hr = CoInitializeEx(NULL,
                        COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        goto Exit;
    }

    fCoInit = TRUE;
    Get(argv[1], &fRetry, &dwCertIgnoreFlags, &cIssuerList, &rgpwszIssuerList);

    if (!fRetry)
    {
        goto Exit;
    }

    Get(argv[1], &fRetry, &dwCertIgnoreFlags, &cIssuerList, &rgpwszIssuerList);

Exit:

    if (fCoInit)
    {
        CoUninitialize();
        fCoInit = FALSE;
    }

    return 0;
}
