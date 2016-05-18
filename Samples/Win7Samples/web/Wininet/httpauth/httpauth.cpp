    // ===========================================================================
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 2000 Microsoft Corporation.  All Rights Reserved.
// ===========================================================================
#define STRSAFE_WITH_GETS
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <wininet.h>
#include <strsafe.h>

//==============================================================================

#define CHECK_ERROR(_cond, _err) \
    do {if (!(_cond)) {pszErr=(_err); goto Done;}} while (FALSE, FALSE)

BOOL NeedAuth(
    HINTERNET       hRequest)
{
    DWORD           dwStatus     = 0;
    DWORD           cbStatus     = sizeof(dwStatus);
    DWORD           dwFlags      = 0;
    BOOL            fRet         = FALSE;
    DWORD           cbScheme     = 0;
    char            szScheme[64] = {};
    DWORD           dwIndex      = 0;

    // Get status code.
    HttpQueryInfo(hRequest,
                  HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
                  &dwStatus,
                  &cbStatus,
                  NULL);
    fprintf(stderr, "Status: %d\n", dwStatus);

    // Look for 401 or 407.
    switch (dwStatus)
    {
    case HTTP_STATUS_DENIED:
        dwFlags = HTTP_QUERY_WWW_AUTHENTICATE;
        break;

    case HTTP_STATUS_PROXY_AUTH_REQ:
        dwFlags = HTTP_QUERY_PROXY_AUTHENTICATE;
        break;

    default:
        fRet = FALSE;
        goto Done;
    }

    // Enumerate the authentication types.
    do
    {
        cbScheme = sizeof(szScheme);
        fRet = HttpQueryInfo(hRequest, dwFlags, szScheme, &cbScheme, &dwIndex);
        if (fRet)
        {
            fprintf(stderr, "Found auth scheme: %s\n", szScheme);
        }
    }
    while (fRet);

Done:
    return fRet;
}


//==============================================================================
DWORD DoCustomUI(
    HINTERNET       hConnect,
    HINTERNET       hRequest)
{
    DWORD           dwError     = ERROR_SUCCESS;
    char            szUser[64]  = {};
    char            szPass[64]  = {};
    BYTE            rgbBuf[1024];
    DWORD           cbBuf       = sizeof(rgbBuf);
    DWORD           cbRead      = 0;

    // Prompt for username and password.

    fprintf(stderr, "Enter Username: ");
    if (FAILED(StringCchGetsA(szUser, ARRAYSIZE(szUser))))
    {
        dwError = ERROR_INTERNET_LOGIN_FAILURE;
        goto Done;
    }

    fprintf(stderr, "Enter Password: ");
    if (FAILED(StringCchGetsA(szPass, ARRAYSIZE(szUser))))
    {
        dwError = ERROR_INTERNET_LOGIN_FAILURE;
        goto Done;
    }

    // Set the values in the handle.
    InternetSetOption(hConnect, INTERNET_OPTION_USERNAME, szUser, sizeof(szUser));
    InternetSetOption(hConnect, INTERNET_OPTION_PASSWORD, szPass, sizeof(szPass));

    // Drain the socket.
    while (InternetReadFile(hRequest, rgbBuf, cbBuf, &cbRead))
    {
        if (cbRead == 0)
        {
            break;
        }
    }

    dwError = ERROR_INTERNET_FORCE_RETRY;

Done:
    return dwError;
}


//==============================================================================
int main(
    int             argc,
    char            **argv)
{
    BOOL            fRet            = FALSE;
    PSTR            pszErr          = NULL;
    DWORD           dwError         = ERROR_SUCCESS;
    BOOL            fAllowCustomUI  = FALSE;
    HINTERNET       hInternet       = NULL;
    HINTERNET       hConnect        = NULL;
    HINTERNET       hRequest        = NULL;
    PSTR            pszHost         = NULL;
    PSTR            pszObject       = NULL;
    PSTR            pszUser         = NULL;
    PSTR            pszPass         = NULL;
    BYTE            rgbBuf[1024];
    DWORD           cbBuf           = sizeof(rgbBuf);
    DWORD           cbRead          = 0;
    DWORD           dwFlags         = 0;

    // Check usage.
    if (argc < 2)
    {
        fprintf(stderr, "Usage:   httpauth [-c] <server> [<object> [<user> [<pass>]]]\n");
        fprintf(stderr, "  -c: Use custom UI to prompt for user/pass");
        exit(1);
    }

    // Parse arguments.
    if (argc >= 2 && strcmp(argv[1], "-c") == 0)
    {
        fAllowCustomUI = TRUE;
        argv++;
        argc--;
    }

    pszHost = argv[1];

    if (argc >= 3)
    {
        pszObject = argv[2];
    }
    else
    {
        pszObject = "/";
    }

    if (argc >= 4)
    {
        pszUser = argv[3];
    }

    if (argc >= 5)
    {
        pszPass = argv[4];
    }

    // Initialize wininet.
    hInternet = InternetOpen("HttpAuth Sample",             // app name
                             INTERNET_OPEN_TYPE_PRECONFIG,  // access type
                             NULL,                          // proxy server
                             0,                             // proxy port
                             0);                            // flags
    CHECK_ERROR(hInternet, "InternetOpen");

    // Connect to host.
    hConnect = InternetConnect(hInternet,               // wininet handle,
                               pszHost,                 // host
                               0,                       // port
                               pszUser,                 // user
                               NULL,                    // pass
                               INTERNET_SERVICE_HTTP,   // service
                               0,                       // flags
                               0);                      // context
    CHECK_ERROR(hConnect, "InternetConnect");

    if (pszPass)
    {
        // Work around InternetConnect disallowing empty passwords.
        InternetSetOption(hConnect,
                          INTERNET_OPTION_PASSWORD,
                          pszPass,
                          lstrlen(pszPass) + 1);
    }

    // Create request.
    dwFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD;
    hRequest = HttpOpenRequest(hConnect,    // connect handle
                               "GET",       // request method
                               pszObject,   // object name
                               NULL,        // version
                               NULL,        // referrer
                               NULL,        // accept types
                               dwFlags,     // flags: keep-alive, bypass cache
                               0);          // context
    CHECK_ERROR(hRequest, "HttpOpenRequest");

    do
    {
        // Send request.
        fRet = HttpSendRequest(hRequest,    // request handle
                               "",          // header string
                               0,           // header length
                               NULL,        // post data
                               0);          // post length

        // Handle any authentication dialogs.
        if (NeedAuth(hRequest) && fAllowCustomUI)
        {
            dwError = DoCustomUI(hConnect, hRequest);
        }
        else
        {
            dwFlags = FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                      FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
                      FLAGS_ERROR_UI_FLAGS_GENERATE_DATA;

            dwError = InternetErrorDlg(GetDesktopWindow(),
                                       hRequest,
                                       fRet ? ERROR_SUCCESS : GetLastError(),
                                       dwFlags,
                                       NULL);
        }
    }
    while (dwError == ERROR_INTERNET_FORCE_RETRY);

    SetLastError(dwError);
    CHECK_ERROR(dwError == ERROR_SUCCESS, "Authentication");

    // Dump some bytes.
    _setmode(_fileno(stdout), _O_BINARY);
    while (InternetReadFile(hRequest, rgbBuf, cbBuf, &cbRead))
    {
        if (cbRead == 0)
        {
            break;
        }

        fwrite(rgbBuf, 1, cbRead, stdout);
    }

Done:

    if (pszErr)
    {
        fprintf(stderr, "Failed on %s, last error %d\n", pszErr, GetLastError());
    }

    // Clean up.

    if (hRequest)
    {
        InternetCloseHandle(hRequest);
        hRequest = NULL;
    }

    if (hConnect)
    {
        InternetCloseHandle(hConnect);
        hConnect = NULL;
    }

    if (hInternet)
    {
        InternetCloseHandle(hInternet);
        hInternet = NULL;
    }

    return 0;
}

