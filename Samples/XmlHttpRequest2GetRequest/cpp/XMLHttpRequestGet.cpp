//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//     This sample demonstrates how to use IXMLHttpRequest2 interface to
//     asynchronously sending HTTP GET requests and receiving HTTP responses.
//
//     XMLHttpRequestGet <Url>
//
//     where:
//
//     Url        is the url for the HTTP request.
//

#include <SDKDDKVer.h>
#include <tchar.h>
#include <msxml6.h>
#include <wrl.h>
#include "XMLHttpRequest2Callback.h"

using namespace Microsoft::WRL;

VOID
Get(
    _In_ PCWSTR pcwszUrl
)
{
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;
    BOOL fAbort = TRUE;
    ComPtr<IXMLHTTPRequest2> spXHR;
    ComPtr<CXMLHttpRequest2Callback> spXhrCallback;

    //
    // Create an object of the IID_IXMLHTTPRequest2 class.
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
    // Create an object of the CXMLHttpRequest2Callback class and initialize it.
    //

    hr = MakeAndInitialize<CXMLHttpRequest2Callback>(&spXhrCallback);
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
    Get(argv[1]);

Exit:

    if (fCoInit)
    {
        CoUninitialize();
        fCoInit = FALSE;
    }

    return 0;
}
