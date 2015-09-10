//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//    Sample WinHttp application for determining the proxy for a particular URL.
//    This sample demonstrates the core functionality for querying the proxy
//    settings.  Additional features may be added by the application/module basing
//    their proxy code from this sample, including but not limited to:
//        1) Per URL proxy cache.
//        2) Network Change awareness.
//        3) Bad Proxy Filter.
//

#include <windows.h>
#include <stdio.h>
#include "GetProxy.h"

#ifdef _CPPUNWIND
#include <new>
#endif

DWORD
SendRequestWithProxyFailover(
    _In_ HINTERNET hRequest,
    _In_ ProxyResolver *pProxyResolver
)
/*++

Routine Description:

    Sends a request using the Request Handle specified and implements
    proxy failover if supported.

Arguments:

    hRequest - The request handle returned by WinHttpOpenRequest.

    pProxyResolver - The Proxy Resolver to use for the request.  The resolver
                     is used to set the proxy infomation on the request handle and
                     to implement proxy failover.  At this point the proxy
                     resolver should have been initialized by calling Resolve().
Return Value:

    Win32 Error codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwRequestError = ERROR_SUCCESS;

    //
    // Reset the proxy list to the beginning in case it is being reused.
    //

    pProxyResolver->ResetProxyCursor();

    for(;;)
    {
        dwError = pProxyResolver->SetNextProxySetting(hRequest,
                                                      dwRequestError);

        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            //
            // We reached the end of the list, failover is not supported,
            // or the error was fatal.
            // Fail with last sendrequest error.
            //

            dwError = dwRequestError;
            goto quit;
        }

        if (dwError != ERROR_SUCCESS)
        {
            //
            // Some other error occured such as a bad proxy setting, bad handle,
            // out of memory, etc.
            //

            goto quit;
        }

        if (!WinHttpSendRequest(hRequest,
                                NULL,                // headers
                                0,                   // headerslen
                                NULL,                // entity
                                0,                   // entitylen
                                0,                   // totallen
                                NULL))               // context
        {
            dwRequestError = GetLastError();
            continue;
        }

        if (!WinHttpReceiveResponse(hRequest,
                                    NULL))           // reserved
        {
            dwRequestError = GetLastError();
            continue;
        }

        dwError = ERROR_SUCCESS;
        break;
    }

quit:

    return dwError;
}

DWORD
SendRequestToHost(
    _In_ HINTERNET hSession,
    _In_ ProxyResolver *pProxyResolver,
    _In_z_ PCWSTR pwszHost,
    _In_opt_z_ PCWSTR pwszPath,
    _Out_ DWORD *pdwStatusCode
)
/*++

Routine Description:

    Connects to a host with the specified proxy and returns the status code
    to the caller.

Arguments:

    hSession - The WinHTTP session to use for the connection.

    pProxyResolver - The proxy resolver for the request.

    pwszHost - The host name of the resource to connect to.

    pwszPath - The path of the resource to connect to.

    pdwStatusCode - The status code of the connection to the server.

Return Value:

    Win32 Error codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwStatusCode = 0;
    DWORD cbStatusCode = sizeof(dwStatusCode);
    DWORD dwFlags = 0;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    PCWSTR pcwszAcceptTypes[] = {L"*/*", NULL};

    *pdwStatusCode = 0;

    //
    // Connect session.
    //

    hConnect = WinHttpConnect(hSession,
                              pwszHost,
                              INTERNET_DEFAULT_HTTP_PORT,
                              0);

    if (hConnect == NULL)
    {
        dwError = GetLastError();
        goto quit;
    }

    //
    //  Open HTTP request.
    //

    hRequest = WinHttpOpenRequest(hConnect,
                                  L"GET",
                                  pwszPath,
                                  NULL,              // version
                                  NULL,              // referrer
                                  pcwszAcceptTypes,
                                  0);                // flags
    if (hRequest == NULL)
    {
        dwError = GetLastError();
        goto quit;
    }

    //
    //  Send the HTTP request with proxy failover if valid.
    //

    dwError = SendRequestWithProxyFailover(hRequest,
                                           pProxyResolver);

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    //
    // Get the status code from the response.
    //

    dwFlags = WINHTTP_QUERY_FLAG_NUMBER | WINHTTP_QUERY_STATUS_CODE;

    if (!WinHttpQueryHeaders(hRequest,
                             dwFlags,
                             NULL,                       // name
                             &dwStatusCode,
                             &cbStatusCode,
                             NULL))                      // index
    {
        dwError = GetLastError();
        goto quit;
    }

    *pdwStatusCode = dwStatusCode;

quit:

    if (hRequest != NULL)
    {
        WinHttpCloseHandle(hRequest);
        hRequest = NULL;
    }

    if (hConnect != NULL)
    {
        WinHttpCloseHandle(hConnect);
        hConnect = NULL;
    }

    return dwError;
}

_Must_inspect_result_
_Success_(return == ERROR_SUCCESS)
DWORD
CrackHostAndPath(
    _In_z_ PCWSTR pwszUrl,
    _Out_ PWSTR *ppwszHost,
    _Out_ PWSTR *ppwszPath
)
/*++

Routine Description:

    Cracks the Host name and Path from a URL and returns the result to the
    caller.

Arguments:

    pwszUrl - The URL to crack.

    ppwszHost - The Host name cracked from the URL.
                Free ppwszHost with free.

    ppwszPath - The Path cracked from the URL or NULL if no path was provided.
                Free ppwszPath with free.

Return Value:

    Win32 Error codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbHost = 0;
    DWORD cbPath = 0;

    PWSTR pwszHost = NULL;
    PWSTR pwszPath = NULL;

    URL_COMPONENTS urlComponents = {};

    //
    // Get the length of each component.
    //

    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.dwHostNameLength = (DWORD)-1;
    urlComponents.dwUrlPathLength = (DWORD)-1;

    if (!WinHttpCrackUrl(pwszUrl,
                         0,
                         0,
                         &urlComponents))
    {
        dwError = GetLastError();
    }

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    //
    // Create a buffer to copy each component.
    //

    cbHost = urlComponents.dwHostNameLength * sizeof(WCHAR) + sizeof(WCHAR);
    pwszHost = (PWSTR)HeapAlloc(GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                cbHost);
    if (pwszHost == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto quit;
    }

    //
    // Account for '\0' in Length.
    //

    pwszHost[0] = '\0';
    urlComponents.dwHostNameLength++;

    if (urlComponents.dwUrlPathLength != 0)
    {
        cbPath = urlComponents.dwUrlPathLength * sizeof(WCHAR) + sizeof(WCHAR);
        pwszPath = (PWSTR)HeapAlloc(GetProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    cbPath);
        if (pwszPath == NULL)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto quit;
        }

        //
        // Account for '\0' in Length.
        //

        pwszPath[0] = '\0';
        urlComponents.dwUrlPathLength++;
    }

    //
    // Copy each component into new buffer.
    //

    urlComponents.lpszHostName = pwszHost;
    urlComponents.lpszUrlPath = pwszPath;

    if (!WinHttpCrackUrl(pwszUrl,
                         0,
                         0,
                         &urlComponents))
    {
        dwError = GetLastError();
    }

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    *ppwszHost = pwszHost;
    pwszHost = NULL;

    *ppwszPath = pwszPath;
    pwszPath = NULL;

quit:

    if (pwszHost != NULL)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pwszHost);

        pwszHost = NULL;
    }

    if (pwszPath != NULL)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pwszPath);

        pwszPath = NULL;
    }

    return dwError;
}

int
__cdecl
wmain(
    int argc,
    _In_reads_(argc) WCHAR **argv
)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwStatusCode = 0;

    PWSTR pwszUrl = NULL;
    PWSTR pwszHost = NULL;
    PWSTR pwszPath = NULL;

    HINTERNET hRequestSession = NULL;
    HINTERNET hProxyResolveSession = NULL;
    ProxyResolver * pProxyResolver = NULL;

    if (argc != 2)
    {
        wprintf(L"Usage: %s <url>\n", argv[0]);
        goto quit;
    }

    pwszUrl = argv[1];

    //
    // Open a session in synchronous mode for http request.
    //

    hRequestSession = WinHttpOpen(USER_AGENT,
                                  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                  WINHTTP_NO_PROXY_NAME,
                                  WINHTTP_NO_PROXY_BYPASS,
                                  0);
    if (hRequestSession == NULL)
    {
        dwError = GetLastError();
        goto quit;
    }

    //
    // Open a session in asynchronous mode for proxy resolve.
    //

    hProxyResolveSession = WinHttpOpen(USER_AGENT,
                                       WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                       WINHTTP_NO_PROXY_NAME,
                                       WINHTTP_NO_PROXY_BYPASS,
                                       WINHTTP_FLAG_ASYNC);
    if (hProxyResolveSession == NULL)
    {
        dwError = GetLastError();
        goto quit;
    }

    //
    // Create a proxy resolver to use for this URL.
    //

#ifdef _CPPUNWIND
    pProxyResolver = new(std::nothrow) ProxyResolver();
#else
    pProxyResolver = new ProxyResolver();
#endif

    if (pProxyResolver == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto quit;
    }

    //
    // Resolve the proxy for the specified URL.
    //

    dwError = pProxyResolver->ResolveProxy(hProxyResolveSession,
                                           pwszUrl);

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    dwError = CrackHostAndPath(pwszUrl,
                               &pwszHost,
                               &pwszPath);

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    //
    // Attempt to connect to the host and retrieve a status code.
    //

    dwError = SendRequestToHost(hRequestSession,
                                pProxyResolver,
                                pwszHost,
                                pwszPath,
                                &dwStatusCode);

    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    wprintf(L"Status: %d\n", dwStatusCode);

quit:

    if (dwError != ERROR_SUCCESS)
    {
        wprintf(L"Failed with Error: 0x%x\n", dwError);
    }

    if (hRequestSession != NULL)
    {
        WinHttpCloseHandle(hRequestSession);
        hRequestSession = NULL;
    }

    if (hProxyResolveSession != NULL)
    {
        WinHttpCloseHandle(hProxyResolveSession);
        hProxyResolveSession = NULL;
    }

    if (pProxyResolver != NULL)
    {
        delete pProxyResolver;
        pProxyResolver = NULL;

    }

    if (pwszHost != NULL)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pwszHost);

        pwszHost = NULL;
    }

    if (pwszPath != NULL)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pwszPath);

        pwszPath = NULL;
    }
}
