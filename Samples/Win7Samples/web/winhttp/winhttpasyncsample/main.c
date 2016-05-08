//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Simple Winhttp async app, with cancellation.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winhttp.h>
#pragma warning(disable:4306)   // conversion from smaller to greater size

#if defined(DBG) || defined(_DEBUG) || defined(DEBUG)
#define ASSERT(x) {if (!(x)) {DebugBreak();}}
#else
#define ASSERT(x)
#endif

HINTERNET g_hSession = NULL;
HINTERNET g_hConnect = NULL;

typedef struct _MYCONTEXT
{
    LONG ReferenceCount;
    CRITICAL_SECTION Lock;
    BOOL LockInitialized;
    HINTERNET RequestHandle;
    HANDLE RequestFinishedEvent;
    DWORD LastError;
    BYTE Buffer[8192];
} MYCONTEXT, *PMYCONTEXT;

VOID
CancelRequest(
    PMYCONTEXT pContext
    );

VOID
FreeMyContext(
    PMYCONTEXT pContext
    )

/*++

Routine Description:

    Frees a context. Generally this should only be called by DereferenceContext.

Arguments:

    pContext - Request context to free.

Return Value:

    None.

--*/

{
    ASSERT(pContext->ReferenceCount == 0);

    if (pContext->LockInitialized)
    {
        DeleteCriticalSection(&pContext->Lock);
        pContext->LockInitialized = FALSE;
    }

    if (pContext->RequestFinishedEvent != NULL)
    {
        CloseHandle(pContext->RequestFinishedEvent);
        pContext->RequestFinishedEvent = NULL;
    }

    HeapFree(GetProcessHeap(), 0, pContext);
}

VOID
ReferenceContext(
    PMYCONTEXT pContext
    )
{
    InterlockedIncrement(&pContext->ReferenceCount);
}

VOID
DereferenceContext(
    PMYCONTEXT pContext
    )
{
    LONG lRefCount;
    lRefCount = InterlockedDecrement(&pContext->ReferenceCount);
    if (lRefCount == 0)
    {
        CancelRequest(pContext);
        FreeMyContext(pContext);
    }
}

DWORD
CreateMyContext(
    HINTERNET Request,
    PMYCONTEXT* ppOutContext
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PMYCONTEXT pContext = NULL;

    *ppOutContext = NULL;

    pContext = (PMYCONTEXT)HeapAlloc(GetProcessHeap(), 
                                     HEAP_ZERO_MEMORY, 
                                     sizeof(MYCONTEXT));
    if (pContext == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }
    pContext->ReferenceCount = 1;

    if (!InitializeCriticalSectionAndSpinCount(&pContext->Lock, 1000))
    {
        dwError = GetLastError();
        goto Exit;
    }
    pContext->LockInitialized = TRUE;

    pContext->RequestFinishedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pContext->RequestFinishedEvent == NULL)
    {
        dwError = GetLastError();
        goto Exit;
    }

    pContext->RequestHandle = Request;
    pContext->LastError = ERROR_SUCCESS;

    *ppOutContext = pContext;
    pContext = NULL;

Exit:

    if (pContext != NULL)
    {
        DereferenceContext(pContext);
        pContext = NULL;
    }

    return dwError;
}

DWORD
LockRequestHandle(
    PMYCONTEXT pContext
    )
{
    DWORD dwError = ERROR_SUCCESS;

    EnterCriticalSection(&pContext->Lock);
    
    if (pContext->RequestHandle == NULL)
    {
        //
        // Request handle is gone already, no point in trying to use it.
        //

        dwError = ERROR_OPERATION_ABORTED;
        LeaveCriticalSection(&pContext->Lock);
    }

    return dwError;
}

VOID
UnlockRequestHandle(
    PMYCONTEXT pContext
    )
{
    LeaveCriticalSection(&pContext->Lock);
}

DWORD
StartReadData(
    PMYCONTEXT pContext
    )
{
    DWORD dwError = ERROR_SUCCESS;

    //
    // Notice how we're under LockRequestHandle, so it's OK to touch 
    // pContext->RequestHandle.
    //

    if (!WinHttpReadData(pContext->RequestHandle,
                         pContext->Buffer, 
                         sizeof(pContext->Buffer),
                         NULL))
    {
        dwError = GetLastError();
        printf("WinHttpReadData failed\n");
        goto Exit;
    }

Exit:

    return dwError;
}

DWORD
OnHeadersAvailable(
    PMYCONTEXT pContext
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwFlags = WINHTTP_QUERY_FLAG_NUMBER | WINHTTP_QUERY_STATUS_CODE;
    DWORD StatusCode = 0;
    DWORD StatusCodeLength = sizeof(StatusCode);

    printf("OnHeadersAvailable\n");

    if (!WinHttpQueryHeaders(pContext->RequestHandle,
                             dwFlags,
                             NULL,
                             &StatusCode,
                             &StatusCodeLength,
                             NULL)) 
    {
        dwError = GetLastError();
        printf("OnHeadersAvailable: WinHttpQueryHeaders failed\n");
        goto Exit;
    }

    printf("OnHeadersAvailable: Status=%d\n", StatusCode);

    dwError = StartReadData(pContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

Exit:

    return dwError;
}

DWORD
OnReadComplete(
    PMYCONTEXT pContext, 
    DWORD dwStatusInformationLength
    )
{
    DWORD dwError = ERROR_SUCCESS;

    printf("OnReadComplete\n");

    if (dwStatusInformationLength != 0)
    {
        printf("OnReadComplete: Bytes read = %d\n", dwStatusInformationLength);
        dwError = StartReadData(pContext);
        if (dwError != ERROR_SUCCESS)
        {
            goto Exit;
        }
    }
    else
    {
        printf("OnReadComplete: Read complete\n");
        SetEvent(pContext->RequestFinishedEvent);
    }

Exit:

    return dwError;
}

VOID
CALLBACK 
AsyncCallback(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL fLocked = FALSE;
    BOOL fReleaseContext = FALSE;
    WINHTTP_ASYNC_RESULT* pWinhttpAsyncResult = NULL;
    PMYCONTEXT pContext = (PMYCONTEXT)dwContext;

    UNREFERENCED_PARAMETER(hInternet);

    if (pContext == NULL)
    {
        //
        // No context, nothing to do.
        //

        goto Exit;
    }

    if (dwInternetStatus != WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING &&
        dwInternetStatus != WINHTTP_CALLBACK_STATUS_REQUEST_ERROR)
    {
        //
        // If we're going to try use the request handle then we'd better lock
        // it.
        //

        dwError = LockRequestHandle(pContext);
        if (dwError != ERROR_SUCCESS)
        {
            goto Exit;
        }
        fLocked = TRUE;
    }

    switch(dwInternetStatus)
    {

    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        printf("AsyncCallback: WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE\n");

        if (!WinHttpReceiveResponse(pContext->RequestHandle, NULL)) 
        {
            dwError = GetLastError();
            printf("AsyncCallback: WinHttpReceiveResponse failed\n");
            goto Exit;
        }
        break;

    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        dwError = OnHeadersAvailable(pContext);
        if (dwError != ERROR_SUCCESS)
        {
            goto Exit;
        }
        break;

    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        dwError = OnReadComplete(pContext, dwStatusInformationLength);
        if (dwError != ERROR_SUCCESS)
        {
            goto Exit;
        }
        break;

    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:

        //
        // Garanteed last callback this context will ever receive. Release 
        // context when we're done on behalf of all callbacks. (Balances the
        // reference we took when we called WinHttpSendRequest)
        //

        fReleaseContext = TRUE;
        break;

    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        printf("AsyncCallback: WINHTTP_CALLBACK_STATUS_REQUEST_ERROR\n");
        pWinhttpAsyncResult = (WINHTTP_ASYNC_RESULT*)lpvStatusInformation;
        dwError = pWinhttpAsyncResult->dwError;
        goto Exit;
        break;

    }

Exit:

    if (dwError != ERROR_SUCCESS) 
    {
        printf("AsyncCallback: dwError = %d\n", dwError);
        if (pContext != NULL)
        {
            pContext->LastError = dwError;
            SetEvent(pContext->RequestFinishedEvent);
        }
    }

    if (fLocked)
    {
        UnlockRequestHandle(pContext);
        fLocked = FALSE;
    }

    if (fReleaseContext)
    {
        DereferenceContext(pContext);
        pContext = NULL;
        fReleaseContext = FALSE;
    }
}

DWORD
InitializeGlobals(
    PCWSTR pwszServer
    )
{
    DWORD dwError = ERROR_SUCCESS;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;

    hSession = WinHttpOpen(L"winhttp async sample/0.1",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS,
                           WINHTTP_FLAG_ASYNC);
    if (hSession == NULL) 
    {
        dwError = GetLastError();
        goto Exit;
    }

    if (WinHttpSetStatusCallback(hSession,
                                 (WINHTTP_STATUS_CALLBACK)AsyncCallback,
                                 WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
                                 0) == WINHTTP_INVALID_STATUS_CALLBACK) 
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

    g_hSession = hSession;
    hSession = NULL;
    g_hConnect = hConnect;
    hConnect = NULL;

Exit:

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

VOID
CleanupGlobals(
    VOID
    )
{
    if (g_hConnect != NULL) 
    {
        WinHttpCloseHandle(g_hConnect);
        g_hConnect = NULL;
    }

    if (g_hSession != NULL) 
    {
        WinHttpCloseHandle(g_hSession);
        g_hSession = NULL;
    }
}

DWORD
BeginRequest(
    PWSTR pwszPath,
    PMYCONTEXT *ppContext
    )

/*++

Routine Description:

    Creates and begins a request.

Arguments:

    pwszPath - Supplies the abs_path to use.

    ppContext - Returns a context, caller should use it as follows:
                1. At least one of EndRequest and CancelRequest.
                2. DereferenceContext

Return Value:

    Win32.

--*/

{
    DWORD dwError = ERROR_SUCCESS;
    HINTERNET hRequest = NULL;
    BOOL fLocked = FALSE;
    PCWSTR pwszAcceptTypes[] = {L"*/*", NULL};
    PMYCONTEXT pContext = NULL;

    *ppContext = NULL;

    hRequest = WinHttpOpenRequest(g_hConnect,
                                  L"GET",
                                  pwszPath,
                                  NULL,              // version
                                  NULL,              // referrer
                                  pwszAcceptTypes,
                                  0);                // flags
    if (hRequest == NULL) 
    {
        dwError = GetLastError();
        goto Exit;
    }

    dwError = CreateMyContext(hRequest, &pContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    //
    // pContext now owns hRequest.
    //

    hRequest = NULL;

    dwError = LockRequestHandle(pContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    fLocked = TRUE;

    //
    // Take an extra reference for async callbacks.
    //

    ReferenceContext(pContext);
    if (!WinHttpSetOption(pContext->RequestHandle,
                          WINHTTP_OPTION_CONTEXT_VALUE,
                          &pContext,
                          sizeof(pContext)))
    {
        dwError = GetLastError();

        //
        // Failed to kick off async work, so no async callbacks, so revoke that
        // reference.
        //

        DereferenceContext(pContext);

        printf("WinHttpSetOption WINHTTP_OPTION_CONTEXT_VALUE failed\n");
        goto Exit;
    }


    if (!WinHttpSendRequest(pContext->RequestHandle,
                            NULL,
                            0,
                            NULL,
                            0,
                            0,
                            0)) 
    {
        dwError = GetLastError();
        printf("WinHttpSendRequest failed\n");
        goto Exit;
    }

    UnlockRequestHandle(pContext);
    fLocked = FALSE;

    //
    // Hand off context ownership to caller.
    //

    *ppContext = pContext;
    pContext = NULL;

Exit:

    if (fLocked)
    {
        UnlockRequestHandle(pContext);
        fLocked = FALSE;
    }

    if (pContext != NULL)
    {
        DereferenceContext(pContext);
        pContext = NULL;
    }

    if (hRequest != NULL)
    {
        WinHttpCloseHandle(hRequest);
        hRequest = NULL;
    }

    return dwError;
}

DWORD
EndRequest(
    PMYCONTEXT pContext
    )

/*++

Routine Description:

    Waits for request to complete.

Arguments:

    pContext - Request context to wait for.

Return Value:

    None.

--*/

{
    DWORD dwError = ERROR_SUCCESS;

    if (WaitForSingleObject(pContext->RequestFinishedEvent, 
                            INFINITE) == WAIT_FAILED)
    {
        dwError = GetLastError();
        printf("WaitForSingleObject failed\n");
        goto Exit;
    }

    dwError = pContext->LastError;

Exit:

    //
    // Success or failure, we're done with this RequestHandle.
    //

    CancelRequest(pContext);

    return dwError;
}

VOID
CancelRequest(
    PMYCONTEXT pContext
    )

/*++

Routine Description:

    A cancel function that is safe to call at any time pContext is valid, from 
    any thread.

Arguments:

    pContext - Request context to cancel.

Return Value:

    None.

--*/

{
    DWORD dwError = ERROR_SUCCESS;
    BOOL fLocked = FALSE;
    HINTERNET hRequest = NULL;

    //
    // This is a short piece of code, but there's a lot going on here:
    // - We do not touch pContext without owning a reference.
    // - We do not use RequestHandle without being under the lock.
    // - We check that the RequestHandle is valid before using. The request may
    //   have finished successfully, or someone else may have cancelled it, 
    //   while we were waiting for the lock. (this check is inside 
    //   LockRequestHandle)
    // - We NULL the RequestHandle before calling WinHttpCloseHandle, cause 
    //   there are cases where winhttp will call back inside WinHttpCloseHandle.
    //

    dwError = LockRequestHandle(pContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    fLocked = TRUE;

    hRequest = pContext->RequestHandle;
    pContext->RequestHandle = NULL;

    WinHttpCloseHandle(hRequest);

Exit:

    if (fLocked)
    {
        UnlockRequestHandle(pContext);
        fLocked = FALSE;
    }
}

DWORD
WINAPI
DemoCancelThreadFunc(
    LPVOID lpParameter
    )

/*++

Routine Description:

    Background thread to demonstrate async cancellation.

Arguments:

    lpParameter - Request context to cancel.

Return Value:

    Thread exit value.

--*/

{
    PMYCONTEXT pContext = (PMYCONTEXT)lpParameter;

    srand((unsigned)time(NULL));
    if (rand() % 2 == 0)
    {
        //
        // Make the cancellation race interesting by sleeping sometimes ..
        //

        printf("DemoCancelThreadFunc sleeping..\n");
        Sleep(100);
    }
    else
    {
        printf("DemoCancelThreadFunc NOT sleeping..\n");
    }

    CancelRequest(pContext);
    DereferenceContext(pContext);
    return ERROR_SUCCESS;
}

DWORD
DemoCancel(
    PMYCONTEXT pContext
    )

/*++

Routine Description:

    Kicks off a DemoCancelThreadFunc thread.

Arguments:

    pContext - Request context to cancel.

Return Value:

    None.

--*/

{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hThread = NULL;

    //
    // Take an additional reference for DemoCancelThreadFunc.
    //

    ReferenceContext(pContext);

    hThread = CreateThread(NULL,
                           0,
                           DemoCancelThreadFunc,
                           pContext,
                           0,
                           NULL);
    if (hThread == NULL)
    {
        dwError = GetLastError();
        DereferenceContext(pContext);
        goto Exit;
    }

Exit:

    if (hThread != NULL)
    {
        CloseHandle(hThread);
        hThread = NULL;
    }

    return dwError;
}

int
__cdecl
wmain(
    int argc,
    wchar_t **argv
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
    PMYCONTEXT pRegularContext = NULL;
    PMYCONTEXT pCancelContext = NULL;

    if (argc != 2)
    {
        printf("Usage: %S <Server>\n", argv[0]);
        goto Exit;
    }

    dwError = InitializeGlobals(argv[1]);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    //
    // Kick off a regular request.
    //

    dwError = BeginRequest(L"/", &pRegularContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    //
    // Kick off a request we actually going to always cancel (for demonstration
    // purposes).
    //

    dwError = BeginRequest(L"/cancel", &pCancelContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    dwError = DemoCancel(pCancelContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    //
    // Wait for the regular request to complete normally.
    //

    dwError = EndRequest(pRegularContext);
    if (dwError != ERROR_SUCCESS)
    {
        goto Exit;
    }

    printf("pRegularContext completed successfully\n");

    //
    // Wait for the cancel request to complete normally.
    //

    dwError = EndRequest(pCancelContext);
    if (dwError != ERROR_SUCCESS)
    {
        if (dwError == ERROR_OPERATION_ABORTED ||
            dwError == ERROR_WINHTTP_OPERATION_CANCELLED)
        {
            printf("DemoCancelThreadFunc won the race and cancelled "
                   "pCancelContext\n");
        }

        goto Exit;
    }

    printf("pCancelContext completed successfully\n");

Exit:

    if (dwError != ERROR_SUCCESS) 
    {
        printf("dwError=%d\n", dwError);
    }

    if (pCancelContext != NULL)
    {
        DereferenceContext(pCancelContext);
        pCancelContext = NULL;
    }

    if (pRegularContext != NULL)
    {
        DereferenceContext(pRegularContext);
        pRegularContext = NULL;
    }

    CleanupGlobals();

    return (int)dwError;
}
