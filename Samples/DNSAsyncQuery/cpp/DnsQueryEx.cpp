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
//     The sample demonstrates how to use asynchronous DnsQueryEx to
//     resolve DNS records as well as using various types of DNS records.
//
//     DnsQuery -q <QueryName> [-t QueryType] [-o QueryOptions] [-s server]
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <windns.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>

//
//  Asynchronous query context structure.
//

typedef struct _QueryContextStruct
{
    ULONG               RefCount;
    WCHAR               QueryName[DNS_MAX_NAME_BUFFER_LENGTH];
    WORD                QueryType;
    ULONG               QueryOptions;
    DNS_QUERY_RESULT    QueryResults;
    DNS_QUERY_CANCEL    QueryCancelContext;
    HANDLE              QueryCompletedEvent;
}QUERY_CONTEXT, *PQUERY_CONTEXT;

DWORD
ParseArguments(
    _In_ int Argc,
    _In_reads_(Argc) PWCHAR Argv[],
    _Out_writes_(DNS_MAX_NAME_BUFFER_LENGTH) PWSTR QueryName,
    _Out_ WORD *QueryType,
    _Out_ ULONG *QueryOptions,
    _Out_writes_(MAX_PATH) PWSTR ServerIp
    );

VOID
PrintDnsRecordList(
    _Inout_ PDNS_RECORD DnsRecord
    );

VOID
AddReferenceQueryContext(
    _Inout_ PQUERY_CONTEXT QueryContext
    );

VOID
DeReferenceQueryContext(
    _Inout_ PQUERY_CONTEXT *QueryContext
    );

DWORD
AllocateQueryContext(
    _Out_ PQUERY_CONTEXT *QueryContext
    );

DWORD
CreateDnsServerList(
    _In_ PWSTR ServerIp,
    _Out_ PDNS_ADDR_ARRAY DnsServerList
    );

VOID
WINAPI
QueryCompleteCallback(
    _In_ PVOID Context,
    _Inout_ PDNS_QUERY_RESULT QueryResults
    );

int
__cdecl
wmain(
    _In_ int Argc,
    _In_reads_(Argc) PWCHAR Argv[]
    )
{
    DWORD Error = ERROR_SUCCESS;
    PQUERY_CONTEXT QueryContext = NULL;
    DNS_QUERY_REQUEST DnsQueryRequest;
    DWORD QueryTimeout = 5 * 1000; // 5 seconds
    WCHAR ServerIp[MAX_PATH];
    DNS_ADDR_ARRAY DnsServerList;

    //
    //  Allocate QueryContext
    //

    Error = AllocateQueryContext(&QueryContext);

    if (Error != ERROR_SUCCESS)
    {
        wprintf(L"AllocateQueryContext failed with error %d", Error);
        goto exit;
    }

    //
    //  Parse input arguments
    //

    Error = ParseArguments(Argc,
                           Argv,
                           QueryContext->QueryName,
                           &QueryContext->QueryType,
                           &QueryContext->QueryOptions,
                           ServerIp);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    //
    //  Initiate asynchronous DnsQuery: Note that QueryResults and
    //  QueryCancelContext should be valid till query completes.
    //

    ZeroMemory(&DnsQueryRequest, sizeof(DnsQueryRequest));

    DnsQueryRequest.Version = DNS_QUERY_REQUEST_VERSION1;
    DnsQueryRequest.QueryName = QueryContext->QueryName;
    DnsQueryRequest.QueryType = QueryContext->QueryType;
    DnsQueryRequest.QueryOptions = (ULONG64)QueryContext->QueryOptions;
    DnsQueryRequest.pQueryContext = QueryContext;
    DnsQueryRequest.pQueryCompletionCallback = QueryCompleteCallback;

    //
    //  If user specifies server, construct DNS_ADDR_ARRAY
    //

    if (ServerIp[0] != L'\0')
    {
        Error = CreateDnsServerList(ServerIp, &DnsServerList);

        if (Error != ERROR_SUCCESS)
        {
            wprintf(L"CreateDnsServerList failed with error %d", Error);
            goto exit;
        }

        DnsQueryRequest.pDnsServerList = &DnsServerList;
    }

    //
    //  Add a reference before initiating asynchronous query.
    //

    AddReferenceQueryContext(QueryContext);

    Error = DnsQueryEx(&DnsQueryRequest,
                       &QueryContext->QueryResults,
                       &QueryContext->QueryCancelContext);

    //
    //  If DnsQueryEx() returns  DNS_REQUEST_PENDING, Completion routine
    //  will be invoked. If not (when completed inline) completion routine
    //  will not be invoked.
    //

    if (Error != DNS_REQUEST_PENDING)
    {
        QueryCompleteCallback(QueryContext, &QueryContext->QueryResults);
        goto exit;
    }

    //
    //  Wait for query completion for 5 seconds and initiate cancel if
    //  completion has not happened.
    //

    if (WaitForSingleObject(QueryContext->QueryCompletedEvent,
                            QueryTimeout)  == WAIT_TIMEOUT)
    {

        //
        //  Initiate Cancel: Note that Cancel is just a request which will
        //  speed the process. It should still wait for the completion callback.
        //

        wprintf(L"The query took longer than %d seconds to complete; "
                L"cancelling the query...\n", QueryTimeout/1000);

        DnsCancelQuery(&QueryContext->QueryCancelContext);

        WaitForSingleObject(QueryContext->QueryCompletedEvent,
                            INFINITE);
    }

exit:

    if (QueryContext)
    {
        DeReferenceQueryContext(&QueryContext);
    }

    return Error;
}

VOID
AddReferenceQueryContext(
    _Inout_ PQUERY_CONTEXT QueryContext
    )
{
    InterlockedIncrement(&QueryContext->RefCount);
}

VOID
DeReferenceQueryContext(
    _Inout_ PQUERY_CONTEXT *QueryContext
    )
{
    PQUERY_CONTEXT QC = *QueryContext;

    if (InterlockedDecrement(&QC->RefCount) == 0)
    {
        if (QC->QueryCompletedEvent)
        {
            CloseHandle(QC->QueryCompletedEvent);
        }

        HeapFree(GetProcessHeap(),
                 0,
                 QC);
        *QueryContext = NULL;
    }
}

DWORD
AllocateQueryContext(
    _Out_ PQUERY_CONTEXT *QueryContext
    )
{
    DWORD Error = ERROR_SUCCESS;

    *QueryContext = (PQUERY_CONTEXT) HeapAlloc(GetProcessHeap(),
                                               HEAP_ZERO_MEMORY,
                                               sizeof(QUERY_CONTEXT));
    if (*QueryContext == NULL)
    {
        Error = GetLastError();
        goto exit;
    }

    AddReferenceQueryContext(*QueryContext);

    (*QueryContext)->QueryResults.Version = DNS_QUERY_RESULTS_VERSION1;

    (*QueryContext)->QueryCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if ((*QueryContext)->QueryCompletedEvent == NULL)
    {
        Error = GetLastError();
        DeReferenceQueryContext(QueryContext);
        *QueryContext = NULL;
        goto exit;
    }

exit:
    return Error;
}

//
//  Wrapper function that creates DNS_ADDR_ARRAY from IP address string.
//

DWORD
CreateDnsServerList(
    _In_ PWSTR ServerIp,
    _Out_ PDNS_ADDR_ARRAY DnsServerList
    )
{
    DWORD  Error = ERROR_SUCCESS;
    SOCKADDR_STORAGE SockAddr;
    INT AddressLength;
    WSADATA wsaData;

    ZeroMemory(DnsServerList, sizeof(*DnsServerList));

    Error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (Error != 0)
    {
        wprintf(L"WSAStartup failed with %d\n", Error);
        return Error;
    }

    AddressLength = sizeof(SockAddr);
    Error = WSAStringToAddress(ServerIp,
                               AF_INET,
                               NULL,
                               (LPSOCKADDR)&SockAddr,
                               &AddressLength);
    if (Error != ERROR_SUCCESS)
    {

        AddressLength = sizeof(SockAddr);
        Error = WSAStringToAddress(ServerIp,
                                   AF_INET6,
                                   NULL,
                                   (LPSOCKADDR)&SockAddr,
                                   &AddressLength);
    }

    if (Error != ERROR_SUCCESS)
    {
        wprintf(L"WSAStringToAddress for %s failed with error %d\n",
                ServerIp,
                Error);
        goto exit;
    }

    DnsServerList->MaxCount = 1;
    DnsServerList->AddrCount = 1;
    CopyMemory(DnsServerList->AddrArray[0].MaxSa, &SockAddr, DNS_ADDR_MAX_SOCKADDR_LENGTH);

exit:

    WSACleanup();
    return Error;
}


//
// Callback function called by DNS as part of asynchronous query complete
//

VOID
WINAPI
QueryCompleteCallback(
    _In_ PVOID Context,
    _Inout_ PDNS_QUERY_RESULT QueryResults
    )
{
    PQUERY_CONTEXT QueryContext = (PQUERY_CONTEXT)Context;

    if (QueryResults->QueryStatus == ERROR_SUCCESS)
    {
        wprintf(L"DnsQuery() succeeded.\n Query response records:\n");
        PrintDnsRecordList(QueryResults->pQueryRecords);
    }
    else
    {
        wprintf(L"DnsQuery() failed, %d\n", QueryResults->QueryStatus);
    }

    if (QueryResults->pQueryRecords)
    {
        DnsRecordListFree(QueryResults->pQueryRecords, DnsFreeRecordList);
    }

    SetEvent(QueryContext->QueryCompletedEvent);
    DeReferenceQueryContext(&QueryContext);
}

