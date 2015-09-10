//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//     This sample demonstrate how to use IXMLHttpRequest2 interface to
//     asynchronously sending HTTP POST requests and receiving HTTP responses.
//
//     XMLHttpRequestPost <Filename> <Url>
//
//     where:
//
//     Filename    is the text file containing the data to be sent.
//     Url         is the url for the HTTP request.
//

#include <SDKDDKVer.h>
#include <tchar.h>
#include <msxml6.h>
#include <wrl.h>
#include "XMLHttpRequest2Callback.h"
#include "XMLHttpRequestPostStream.h"

using namespace Microsoft::WRL;

VOID
Post(
    _In_ PCWSTR pcwszFileName,
    _In_ PCWSTR pcwszUrl
)
{
    HRESULT hr = S_OK;
    DWORD dwStatus = 0;
    ULONGLONG ullFileSize = 0;
    BOOL fAbort = TRUE;
    ComPtr<IXMLHTTPRequest2> spXHR;
    ComPtr<CXMLHttpRequest2Callback> spXhrCallback;
    ComPtr<CXMLHttpRequestPostStream> spXhrPostStream;

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
    // Create an object of the CXMLHttpRequest2Callback class.
    //

    hr = MakeAndInitialize<CXMLHttpRequest2Callback>(&spXhrCallback);
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Create an object of the CXMLHttpRequestPostStream class.
    //

    spXhrPostStream = Make<CXMLHttpRequestPostStream>();
    if (spXhrPostStream == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    //
    // Open and get the file size of the post file.
    //

    hr = spXhrPostStream->Open(pcwszFileName);
    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = spXhrPostStream->GetSize(&ullFileSize);
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Send a HTTP POST request.
    //

    hr = spXHR->Open(L"POST",             // Method.
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

    hr = spXHR->Send(spXhrPostStream.Get(), ullFileSize);
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

    if (argc != 3)
    {
        wprintf(L"Usage: %s <Filename> <Url>\n", argv[0]);
        goto Exit;
    }

    hr = CoInitializeEx(NULL,
                        COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        goto Exit;
    }

    fCoInit = TRUE;
    Post(argv[1], argv[2]);

Exit:

    if (fCoInit)
    {
        CoUninitialize();
        fCoInit = FALSE;
    }

    return 0;
}
