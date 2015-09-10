//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Abstract:
//
//     The sample demonstrates how to use asynchronous GetAddrInfoEx to
//     resolve name to IP address.
//
//     ResolveName <QueryName>
//
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <Ws2tcpip.h>

#define MAX_ADDRESS_STRING_LENGTH   64

//
//  Asynchronous query context structure.
//

typedef struct _QueryContext
{
    OVERLAPPED      QueryOverlapped;
    PADDRINFOEX     QueryResults;
    HANDLE          CompleteEvent;
}QUERY_CONTEXT, *PQUERY_CONTEXT;

VOID
WINAPI
QueryCompleteCallback(
    _In_ DWORD Error,
    _In_ DWORD Bytes,
    _In_ LPOVERLAPPED Overlapped
    );

int
__cdecl
wmain(
    _In_ int Argc,
    _In_reads_(Argc) PWCHAR Argv[]
    )
{
    INT                 Error = ERROR_SUCCESS;
    WSADATA             wsaData;
    BOOL                IsWSAStartupCalled = FALSE;
    ADDRINFOEX          Hints;
    QUERY_CONTEXT       QueryContext;
    HANDLE              CancelHandle = NULL;
    DWORD               QueryTimeout = 5 * 1000; // 5 seconds

    ZeroMemory(&QueryContext, sizeof(QueryContext));

    //
    //  Validate the parameters
    //

    if (Argc != 2)
    {
        wprintf(L"Usage: ResolveName <QueryName>\n");
        goto exit;
    }

    //
    //  All Winsock functions require WSAStartup() to be called first
    //

    Error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (Error != 0)
    {
        wprintf(L"WSAStartup failed with %d\n", Error);
        goto exit;
    }

    IsWSAStartupCalled = TRUE;

    ZeroMemory(&Hints, sizeof(Hints));
    Hints.ai_family = AF_UNSPEC;

    //
    //  Note that this is a simple sample that waits/cancels a single
    //  asynchronous query. The reader may extend this to support
    //  multiple asynchronous queries.
    //

    QueryContext.CompleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (QueryContext.CompleteEvent == NULL)
    {
        Error = GetLastError();
        wprintf(L"Failed to create completion event: Error %d\n",  Error);
        goto exit;
    }

    //
    //  Initiate asynchronous GetAddrInfoExW.
    //
    //  Note GetAddrInfoEx can also be invoked asynchronously using an event
    //  in the overlapped object (Just set hEvent in the Overlapped object
    //  and set NULL as completion callback.)
    //
    //  This sample uses the completion callback method.
    //

    Error = GetAddrInfoExW(Argv[1],
                           NULL,
                           NS_DNS,
                           NULL,
                           &Hints,
                           &QueryContext.QueryResults,
                           NULL,
                           &QueryContext.QueryOverlapped,
                           QueryCompleteCallback,
                           &CancelHandle);

    //
    //  If GetAddrInfoExW() returns  WSA_IO_PENDING, GetAddrInfoExW will invoke
    //  the completion routine. If GetAddrInfoExW returned anything else we must
    //  invoke the completion directly.
    //

    if (Error != WSA_IO_PENDING)
    {
        QueryCompleteCallback(Error, 0, &QueryContext.QueryOverlapped);
        goto exit;
    }

    //
    //  Wait for query completion for 5 seconds and cancel the query if it has
    //  not yet completed.
    //

    if (WaitForSingleObject(QueryContext.CompleteEvent,
                            QueryTimeout)  == WAIT_TIMEOUT )
    {

        //
        //  Cancel the query: Note that the GetAddrInfoExCancelcancel call does
        //  not block, so we must wait for the completion routine to be invoked.
        //  If we fail to wait, WSACleanup() could be called while an
        //  asynchronous query is still in progress, possibly causing a crash.
        //

        wprintf(L"The query took longer than %d seconds to complete; "
                L"cancelling the query...\n", QueryTimeout/1000);

        GetAddrInfoExCancel(&CancelHandle);

        WaitForSingleObject(QueryContext.CompleteEvent,
                            INFINITE);
    }

exit:

    if (IsWSAStartupCalled)
    {
        WSACleanup();
    }

    if (QueryContext.CompleteEvent)
    {
        CloseHandle(QueryContext.CompleteEvent);
    }

    return Error;
}

//
// Callback function called by Winsock as part of asynchronous query complete
//

VOID
WINAPI
QueryCompleteCallback(
    _In_ DWORD Error,
    _In_ DWORD Bytes,
    _In_ LPOVERLAPPED Overlapped
    )
{
    PQUERY_CONTEXT  QueryContext = NULL;
    PADDRINFOEX     QueryResults = NULL;
    WCHAR           AddrString[MAX_ADDRESS_STRING_LENGTH];
    DWORD           AddressStringLength;

    UNREFERENCED_PARAMETER(Bytes);

    QueryContext = CONTAINING_RECORD(Overlapped,
                                     QUERY_CONTEXT,
                                     QueryOverlapped);

    if (Error != ERROR_SUCCESS)
    {
        wprintf(L"ResolveName failed with %d\n", Error);
        goto exit;
    }

    wprintf(L"ResolveName succeeded. Query Results:\n");

    QueryResults = QueryContext->QueryResults;

    while(QueryResults)
    {
        AddressStringLength = MAX_ADDRESS_STRING_LENGTH;

        WSAAddressToString(QueryResults->ai_addr,
                           (DWORD)QueryResults->ai_addrlen,
                           NULL,
                           AddrString,
                           &AddressStringLength);

        wprintf(L"Ip Address: %s\n", AddrString);
        QueryResults = QueryResults->ai_next;
    }

exit:

    if (QueryContext->QueryResults)
    {
        FreeAddrInfoEx(QueryContext->QueryResults);
    }

    //
    //  Notify caller that the query completed
    //

    SetEvent(QueryContext->CompleteEvent);
    return;
}

