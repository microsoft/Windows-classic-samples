#ifndef UNICODE
#define UNICODE
#endif
#include "Windows.h"
#include "WinHttp.h"
#include "stdio.h"

static const char postData[] = "hello";
static const ULONG postDataLength = sizeof(postData) - 1;

static const WCHAR contentTypeHeader[] = L"Content-Type: text/plain";

HRESULT ReadHeader(
    __in HINTERNET requestHandle, 
    __in_z WCHAR* headerName, 
    __in ULONG queryFlags)
{
    HRESULT hr = S_OK;
    WCHAR headerBuffer[256];
    ULONG headerLength = sizeof(headerBuffer);

    if (!WinHttpQueryHeaders(
        requestHandle,
        queryFlags,
        NULL,
        headerBuffer,
        &headerLength,
        WINHTTP_NO_HEADER_INDEX))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    wprintf(L"%s: %s\n", headerName, headerBuffer);

Exit:
    return hr;
}

HRESULT ReadResponse(
    __in HINTERNET requestHandle)
{
    HRESULT hr = S_OK;
    BYTE buffer[256];
    ULONG bytesRead;

    hr = ReadHeader(requestHandle, L"StatusCode", WINHTTP_QUERY_STATUS_CODE);
    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = ReadHeader(requestHandle, L"StatusText", WINHTTP_QUERY_STATUS_TEXT);
    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = ReadHeader(requestHandle, L"Content-Type", WINHTTP_QUERY_CONTENT_TYPE);
    if (FAILED(hr))
    {
        goto Exit;
    }

    for (;;)
    {
        if (!WinHttpReadData(requestHandle, buffer, sizeof(buffer), &bytesRead))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
        if (bytesRead == 0)
        {
            break;
        }
        wprintf(L"%.*S", bytesRead, buffer);
    }

    wprintf(L"\n");

Exit:
    return hr;
}

HRESULT PerformRequest(
    __in HINTERNET connectionHandle, 
    __in BOOL post)
{
    HRESULT hr = S_OK;
    HANDLE requestHandle = NULL;

    // Open a request
    requestHandle = WinHttpOpenRequest(
        connectionHandle,
        post ? L"POST" : L"GET",
        L"/example",
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0);
    if (requestHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Send the request
    if (!WinHttpSendRequest(
        requestHandle,
        post ? contentTypeHeader : WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        post ? const_cast<char*>(postData) : WINHTTP_NO_REQUEST_DATA,
        post ? postDataLength : 0,
        post ? postDataLength : 0,
        NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Receive the response
    if (!WinHttpReceiveResponse(requestHandle, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Read and print the response
    hr = ReadResponse(requestHandle);
    if (FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:
    if (requestHandle != NULL)
    {
        WinHttpCloseHandle(requestHandle);
    }

    return hr;
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    HINTERNET sessionHandle = NULL;
    HINTERNET connectionHandle = NULL;
    HINTERNET requestHandle = NULL;
    
    
    
    // Get a session handle
    sessionHandle = WinHttpOpen(
        L"", 
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (sessionHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Get a connection handle
    connectionHandle = WinHttpConnect(
        sessionHandle,
        L"localhost",
        80,
        0);
    if (connectionHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Perform a POST request
    hr = PerformRequest(connectionHandle, TRUE);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Perform a GET request
    hr = PerformRequest(connectionHandle, FALSE);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (requestHandle != NULL)
    {
        WinHttpCloseHandle(requestHandle);
    }
    if (connectionHandle != NULL)
    {
        WinHttpCloseHandle(connectionHandle);
    }
    if (sessionHandle != NULL)
    {
        WinHttpCloseHandle(sessionHandle);
    }
    
    
    if (FAILED(hr))
    {
        wprintf(L"Error 0x%08lx\n", hr);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}

