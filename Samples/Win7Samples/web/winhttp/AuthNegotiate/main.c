//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

/*++

Abstract:

    Simple Winhttp app that does the following:
    - Negotiate server auth
    - Synchronous POST
    - SspiPromptForCredentials

HTTP server setup:
    - Negotiate auth
    - A URL that accepts POSTs

--*/

#define REQUEST_ENTITY_SIZE 12345

#include <windows.h>

#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>
#include <wincred.h>

#include <stdio.h>
#include <stdlib.h>

#include <winhttp.h>

//
// BUGBUG: SspiPromptForCredentialsW should not need to be declared for Win7 SDK
//         RTM.
//

#ifdef __cplusplus
extern "C"
#endif
DWORD
SEC_ENTRY
SspiPromptForCredentialsW(
    __in PCWSTR pszTargetName,
    __in_opt PVOID pUiInfo,
    __in DWORD dwAuthError,
    __in PCWSTR pszPackage,
    __in_opt PSEC_WINNT_AUTH_IDENTITY_OPAQUE pInputAuthIdentity,
    __deref_out PSEC_WINNT_AUTH_IDENTITY_OPAQUE* ppAuthIdentity,
    __inout_opt PBOOL pfSave,
    __in DWORD dwFlags
    );

DWORD
SendReceive(
    HINTERNET hRequest,
    DWORD dwExpectedStatusCode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbWritten = 0;
    DWORD dwStatusCode = 0;
    DWORD cbStatusCode = sizeof(dwStatusCode);
    BYTE Buffer[1024] = {0};
    DWORD cbRead = 0;
    DWORD cbRemaining = 0;

    //
    // Send request.
    //

WinhttpResendRequest:

    cbRemaining = REQUEST_ENTITY_SIZE;

    if (WinHttpSendRequest(hRequest,
                           NULL,                // headers
                           0,                   // headerslen
                           NULL,                // entity
                           0,                   // entitylen
                           REQUEST_ENTITY_SIZE, // totallen
                           0) == FALSE)
    {
        dwError = GetLastError();
        printf("WinHttpSendRequest failed\n");
        goto Exit;
    }

    memset(Buffer, 'a', sizeof(Buffer));

    while (cbRemaining > 0) 
    {
        if (WinHttpWriteData(hRequest,
                             Buffer,
                             cbRemaining < sizeof(Buffer) ?
                                cbRemaining : sizeof(Buffer),
                             &cbWritten) == FALSE) 
        {
            dwError = GetLastError();
            goto Exit;
        }

        cbRemaining -= cbWritten;
    }

    //
    // Receive response.
    //

    if (WinHttpReceiveResponse(hRequest, NULL) == FALSE) 
    {
        dwError = GetLastError();

        if (dwError == ERROR_WINHTTP_RESEND_REQUEST) 
        {
            printf("WinHttpReceiveResponse failed with"
                   " ERROR_WINHTTP_RESEND_REQUEST\n");
            dwError = ERROR_SUCCESS;
            goto WinhttpResendRequest;
        }

        printf("WinHttpReceiveResponse failed\n");
        goto Exit;
    }

    if (WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_FLAG_NUMBER | 
                            WINHTTP_QUERY_STATUS_CODE,
                            NULL,
                            &dwStatusCode,
                            &cbStatusCode,
                            NULL) == FALSE) 
    {
        dwError = GetLastError();
        printf("WinHttpQueryHeaders failed\n");
        goto Exit;
    }

    printf("Status=%d\n", dwStatusCode);
    if (dwStatusCode != dwExpectedStatusCode) 
    {
        printf("\n\n*** Unexpected Status Code ***\n\n");
    }

    //
    // Print first 100 bytes of response.
    //

    if (WinHttpReadData(hRequest,
                        Buffer,
                        sizeof(Buffer),
                        &cbRead) == FALSE) 
    {
        dwError = GetLastError();
        printf("WinHttpReadData failed\n");
        goto Exit;
    }

    if (cbRead > 100) 
    {
        cbRead = 100;
    }

    if (cbRead >= sizeof(Buffer)) 
    {
        cbRead = sizeof(Buffer) - 1;
    }

    Buffer[cbRead] = '\0';

    printf("Response: %s\n\n", Buffer);

    //
    // Drain the rest of the response.
    //

    while (cbRead != 0) 
    {
        if (WinHttpReadData(hRequest,
                            Buffer,
                            sizeof(Buffer),
                            &cbRead) == FALSE) 
        {
            dwError = GetLastError();
            printf("WinHttpReadData failed\n");
            goto Exit;
        }
    }

Exit:

    return dwError;
}

int
__cdecl
wmain (
    __in int argc,
    __in_ecount(argc) wchar_t **argv
    )

/*++

Routine Description:

    main

Arguments:

    argc -

    argv -

Return Value:

    Win32.

--*/

{
    DWORD dwError = ERROR_SUCCESS;
    PWSTR pwszServer = NULL;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    PCWSTR pwszAcceptTypes[] = {L"*/*", NULL};
    DWORD dwAutoLogonHigh = WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;

    PSEC_WINNT_AUTH_IDENTITY_OPAQUE pAuthIdentityOpaque = NULL;
    CREDUI_INFO CredUiInfo = {0};

    DWORD dwSupportedSchemes = 0;
    DWORD dwFirstScheme = 0;
    DWORD dwAuthTarget = 0;
    PWSTR pwszSpnUsed = NULL;
    DWORD cbSpnUsed = 0;

    if (argc != 2)
    {
        printf("Usage: %S <Server>\n", argv[0]);
        goto Exit;
    }

    pwszServer = argv[1];

    hSession = WinHttpOpen(L"winhttp sample program/0.1",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS,
                           0);
    if (hSession == NULL) 
    {
        dwError = GetLastError();
        goto Exit;
    }

    hConnect = WinHttpConnect(hSession,
                              pwszServer,
                              INTERNET_DEFAULT_HTTP_PORT,
                              0);
    if (hConnect == NULL)
    {
        dwError = GetLastError();
        goto Exit;
    }

    hRequest = WinHttpOpenRequest(hConnect,
                                  L"GET",
                                  L"/countbytes.aspx",  
                                  NULL,
                                  NULL,
                                  pwszAcceptTypes,
                                  0);
    if (hRequest == NULL) 
    {
        dwError = GetLastError();
        goto Exit;
    }

    //
    // Set autologon to high, so that we don't automatically logon or use
    // default credentials.
    //
    
    if (WinHttpSetOption(hRequest,
                         WINHTTP_OPTION_AUTOLOGON_POLICY,
                         &dwAutoLogonHigh,
                         sizeof(dwAutoLogonHigh)) == FALSE) 
    {
        dwError = GetLastError();
        printf("WinHttpSetOption autologon failed\n");
        goto Exit;
    }

    //
    // Send and receive. Since we know what the server is doing we expect 401 + 
    // Negotiate.
    //

    dwError = SendReceive(hRequest, 401);
    if (dwError != ERROR_SUCCESS)
    {
        printf("SendReceive 1 failed\n");
        goto Exit;
    }

    if (WinHttpQueryAuthSchemes(hRequest,
                                &dwSupportedSchemes,
                                &dwFirstScheme,
                                &dwAuthTarget) == FALSE) 
    {
        dwError = GetLastError();
        printf("WinHttpQueryAuthSchemes failed\n");
        goto Exit;
    }

    if (dwAuthTarget != WINHTTP_AUTH_TARGET_SERVER ||
        (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE) == 0) 
    {
        printf("Expected target=server and scheme=negotiate, bailing ...\n");
        goto Exit;
    } 

    //
    // InitializeSecurityContext and SspiPromptForCredentials have a behind the
    // scenes relationship keyed off SPN for contextual prompting.
    //

    if (WinHttpQueryOption(hRequest,
                           WINHTTP_OPTION_SERVER_SPN_USED,
                           NULL,
                           &cbSpnUsed))
    {
        printf("WinHttpQueryOption WINHTTP_OPTION_SERVER_SPN_USED should have "
               "failed\n");
        dwError = ERROR_INVALID_STATE;
        goto Exit;
    }

    dwError = GetLastError();
    if (dwError == ERROR_INSUFFICIENT_BUFFER) 
    {
        pwszSpnUsed = (PWSTR)HeapAlloc(GetProcessHeap(), 0, cbSpnUsed);
        if (pwszSpnUsed == NULL) 
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }

        if (WinHttpQueryOption(hRequest,
                               WINHTTP_OPTION_SERVER_SPN_USED,
                               pwszSpnUsed,
                               &cbSpnUsed) == FALSE) 
        {
            dwError = GetLastError();
            printf("WINHTTP_OPTION_SERVER_SPN_USED 2 failed\n");
            goto Exit;
        }

    }
    dwError = ERROR_SUCCESS;

    if (pwszSpnUsed == NULL)
    {
        printf("SPN used unavailable, using server name (%S)\n",
               pwszServer);
        pwszSpnUsed = pwszServer;
    }
    else
    {
        printf("SPN used = %S\n", pwszSpnUsed);
    }

    CredUiInfo.cbSize = sizeof(CredUiInfo);
    CredUiInfo.hwndParent = NULL;
    CredUiInfo.pszMessageText = L"Sample Message Text";
    CredUiInfo.pszCaptionText = L"Sample Caption Text";

    dwError = SspiPromptForCredentialsW(pwszSpnUsed,
                                       &CredUiInfo,
                                       0,
                                       L"Negotiate",
                                       NULL,
                                       &pAuthIdentityOpaque,
                                       NULL,
                                       0);
    if (dwError != ERROR_SUCCESS) 
    {
        printf("SspiPromptForCredentials failed\n");
        goto Exit;
    }

    //
    // Only WinHttpSetCredentials(Negotiate) supports receiving a 
    // pAuthIdentityOpaque. (pwszUserName and pwszPassword must be NULL)
    //

    if (WinHttpSetCredentials(hRequest,
                              WINHTTP_AUTH_TARGET_SERVER,
                              WINHTTP_AUTH_SCHEME_NEGOTIATE,
                              NULL,
                              NULL,
                              pAuthIdentityOpaque) == FALSE) 
    {
        dwError = GetLastError();
        printf("WinHttpSetCredentials failed\n");
        goto Exit;
    }

    //
    // After all that work we'd better get a 200! :-)
    //

    dwError = SendReceive(hRequest, 200);
    if (dwError != ERROR_SUCCESS) 
    {
        printf("SendReceive 2 failed\n");
        goto Exit;
    }

Exit:

    if (dwError != ERROR_SUCCESS) 
    {
        printf("dwError=%d\n", dwError);
    }

    if (pAuthIdentityOpaque != NULL) 
    {
        //
        // BUGBUG: SspiFreeAuthIdentity should be uncommented in the Win7 RTM
        //         SDK
        //
        // SspiFreeAuthIdentity(pAuthIdentityOpaque);
        pAuthIdentityOpaque = NULL;
    }

    if (pwszSpnUsed != NULL &&
        pwszSpnUsed != pwszServer) 
    {
        HeapFree(GetProcessHeap(), 0, pwszSpnUsed);
        pwszSpnUsed = NULL;
    }

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

    if (hSession != NULL)
    {
        WinHttpCloseHandle(hSession);
        hSession = NULL;
    }

    return dwError;
}
