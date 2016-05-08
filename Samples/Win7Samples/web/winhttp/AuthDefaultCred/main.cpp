//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Simple Winhttp application using default credentials to 
// authenticate with server that require NTLM authentication
// WARNING: It is not secure to use default credentials on an untrusted
//        server/network or internet server.

#include <windows.h>
#include <stdio.h>
#include <winhttp.h>

#define USER_AGENT L"AuthDefaultCredSample/1.0"
#define TARGET_PATH L"/"

int
__cdecl
wmain(
    int argc,
    WCHAR **argv
)
{
    DWORD dwError = ERROR_SUCCESS;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);
    DWORD dwAutoLogonPolicy = WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW;
    PWSTR pwszServerName = NULL;

    if (argc != 2)
    {
        wprintf(L"Usage: %s <servername>\n", argv[0]);
        goto Exit;
    }

    pwszServerName = argv[1];

    // Use WinHttpOpen to obtain a session handle and specify no proxy  
    hSession = WinHttpOpen(USER_AGENT,
                           WINHTTP_ACCESS_TYPE_NO_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS,
                           0);
    if (hSession == NULL)
    {
        dwError = GetLastError();
        wprintf(L"WinHttpOpen failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpConnect to specify target server and port
    hConnect = WinHttpConnect(hSession,
                              pwszServerName,
                              INTERNET_DEFAULT_HTTP_PORT,
                              0);
    if (hConnect == NULL)
    {
        dwError = GetLastError();
        wprintf(L"WinHttpConnect failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpOpenRequest to open a GET request and specify taget path
    hRequest = WinHttpOpenRequest(hConnect,
                                  L"GET",
                                  TARGET_PATH,
                                  NULL,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  0);
    if (hRequest == NULL)
    {
        dwError = GetLastError();
        wprintf(L"WinHttpOpenRequest failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpSetOption to set autologon policy to low level
    // to include the default credentials in a request
    if (!WinHttpSetOption(hRequest,
                          WINHTTP_OPTION_AUTOLOGON_POLICY,
                          &dwAutoLogonPolicy,
                          sizeof(dwAutoLogonPolicy)))
    {
        dwError = GetLastError();
        wprintf(L"WinHttpSetOption failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpSetCredentials with NULL username and password
    // to use default credentials
    if (!WinHttpSetCredentials(hRequest,
                               WINHTTP_AUTH_TARGET_SERVER,
                               WINHTTP_AUTH_SCHEME_NTLM,
                               NULL,
                               NULL,
                               NULL))
    {
        dwError = GetLastError();
        wprintf(L"WinHttpSetCredentials failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpSendRequest to send the request and
    // specify no additional headers and request data
    if (!WinHttpSendRequest(hRequest,
                            WINHTTP_NO_ADDITIONAL_HEADERS,
                            0,
                            WINHTTP_NO_REQUEST_DATA, 
                            0,
                            0, 
                            0))
    {
        dwError = GetLastError();
        wprintf(L"WinHttpSendRequest failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpReceiveResponse to receive the response
    if (!WinHttpReceiveResponse(hRequest,
                                NULL))
    {
        dwError = GetLastError();
        wprintf(L"WinHttpReceiveResponse failed with error %d\n", dwError);
        goto Exit;
    }

    // Use WinHttpQueryHeaders to retrieve the status code
    if (!WinHttpQueryHeaders(hRequest,
                             WINHTTP_QUERY_FLAG_NUMBER |
                             WINHTTP_QUERY_STATUS_CODE,
                             NULL,
                             &dwStatusCode,
                             &dwSize,
                             NULL))
    {
        dwError = GetLastError();
        wprintf(L"WinHttpQueryHeaders failed with error %d\n", dwError);
        goto Exit;
    }

    // Expect to get status code 200
    if(dwStatusCode == 200)  
    {
        wprintf(L"Got expected status code=200\n");
    }
    else
    {
        wprintf(L"Unexpected status code=%d\n", dwStatusCode);
    }

Exit:
    if (hRequest != NULL)
    {
        WinHttpCloseHandle(hRequest);
    }

    if (hConnect != NULL)
    {
        WinHttpCloseHandle(hConnect);
    }

    if (hSession != NULL)
    {
        WinHttpCloseHandle(hSession);
    }

    return dwError;
}

