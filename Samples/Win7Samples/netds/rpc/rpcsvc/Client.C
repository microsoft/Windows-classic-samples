// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


//  MODULE:   client.c
//
//  PURPOSE:  This program is a command line oriented
//            demonstration of the Simple RPC service sample.
//
//  FUNCTIONS:
//    main(int argc, char **argv);
//    StartTime()
//    EndTime();
//    DoTimings();
//
//  COMMENTS:
//
//  AUTHOR:
//      Mario Goertzel - RPC Development
//
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc.h>
#include "rpcsvc.h"

void Usage(void)
{
    printf("Usage:\n"
        "\t-n <server addr>  - Defaults to local machine\n"
        "\t-t <protseq>      - Defaults to ncalrpc (fast, local only)\n"
        "\t-i <iterations>   - Defaults to 100\n"
        "\t-s <security lvl> - Default none, range none (1) to privacy (6)\n"
        );
    return;
}


//
//  FUNCTIONS: StartTime()
//             EndTime()
//
//  USAGE:
//      StartTime();
//        // Do some work.
//      mseconds = EndTime();
//
//  RETURN VALUE:
//      Milliseconds between StartTime() and EndTime() calls.

LARGE_INTEGER Time;

void StartTime(void)
{
    QueryPerformanceCounter(&Time);
}

ULONG EndTime()
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liFreq;

    QueryPerformanceCounter(&liDiff);

    liDiff.QuadPart -= Time.QuadPart;
    liDiff.QuadPart *= 1000; // Adjust to milliseconds, shouldn't overflow...

    (void)QueryPerformanceFrequency(&liFreq);

    return ((ULONG)(liDiff.QuadPart / liFreq.QuadPart));
}

//
//  FUNCTION: DoTimings
//
//  PURPOSE: Calls and times various RPC calls.
//           (Avoid cluttering up main())
//
//  PARAMETERS:
//    Binding     - Binding to the server.
//    iIterations - Number of times to make each call.
//
//  RETURN VALUE:
//    n/a
//
//
void DoTimings(RPC_BINDING_HANDLE Binding,
               UINT iIterations)
{
    ULONG mseconds;
    UINT i;
    RPC_STATUS status;
    byte bBuffer[4096];
    ULONG lBufferLength;
    ULONG lBufferSize;

    // Time Pings() (void calls)

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = Ping(Binding);
        if (status != RPC_S_OK)
            goto Cleanup;
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - void calls.\n", iIterations, mseconds);

    // Time [in] buffer's
    //

    lBufferLength = BUFFER_SIZE;
    lBufferSize   = BUFFER_SIZE;
    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferIn1(Binding, bBuffer, lBufferLength, lBufferSize);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer in (1).\n", iIterations, mseconds);

    lBufferLength = BUFFER_SIZE;

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferIn3(Binding, bBuffer, lBufferLength);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer in (2).\n", iIterations, mseconds);

    lBufferLength = BUFFER_SIZE;

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferIn3(Binding, bBuffer, lBufferLength);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer in (3).\n", iIterations, mseconds);

    // Time [out] buffer's

    lBufferLength = BUFFER_SIZE;

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferOut1(Binding, bBuffer, &lBufferLength);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer out (1).\n", iIterations, mseconds);

    lBufferLength = BUFFER_SIZE;
    lBufferSize   = BUFFER_SIZE;

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferOut2(Binding, bBuffer, lBufferSize, &lBufferLength);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer out (2).\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        BUFFER Buffer;
        Buffer.BufferLength = 0;
        Buffer.Buffer = 0;

        status = BufferOut3(Binding, &Buffer);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        MIDL_user_free(Buffer.Buffer);
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer out (3).\n", iIterations, mseconds);

    lBufferLength = BUFFER_SIZE;

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = BufferOut4(Binding, bBuffer, &lBufferLength);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 100 byte buffer out (4).\n", iIterations, mseconds);

    // Time arrays of structures

    {
    struct BAD1 abad1[50];
    struct BAD2 abad2[50];
    struct GOOD agood[50];

    for (i = 0; i < 50; i++)
        {
        abad2[i].e = (BAD_ENUM)i % 4 + 1;
        agood[i].e = (GOOD_ENUM)i % 4 + 5;
        }

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = StructsIn1(Binding, &abad1[0]);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - 2 mod 4 aligned structs.\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = StructsIn2(Binding, &abad2[0]);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - structs with an enum.\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = StructsIn3(Binding, &agood[0]);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - structs with v1_enum.\n", iIterations, mseconds);
    }

    // Linked lists

    {
    LIST list;
    PLIST plist = &list;
    for (i = 0; i < LIST_SIZE - 1; i++)
        {
        plist->pNext = MIDL_user_allocate(sizeof(LIST));
        plist->data = i;
        if (plist->pNext == 0)
            {
            status = RPC_S_OUT_OF_MEMORY;
            goto Cleanup;
            }
        plist = plist->pNext;
        }
    plist->data = i;
    plist->pNext = 0;
    

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = ListIn(Binding, &list);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - [in] linked list.\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = ListOut1(Binding, &list);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        // Freeing the list here would cause all the elements
        // to be allocated again on the next call.
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - [out] linked list (1).\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = ListOut2(Binding, &list);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        // Freeing the list here would cause all the elements
        // to be allocated again on the next call.
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - [out] linked list (2).\n", iIterations, mseconds);

    // Free allocated elements of the list.
    plist = list.pNext;
    while(plist)
        {
        PLIST tmp = plist;
        plist = plist->pNext;
        MIDL_user_free(tmp);
        }
    }

    // Unions

    {
    BAD_UNION badunionArray[UNION_ARRAY_LEN];
    GOOD_UNION goodunion;
    ARM_ONE armone;
    ULONG ulArray[UNION_ARRAY_LEN];

    goodunion.Tag = 1;
    goodunion.u.pOne = &armone;
    armone.DataLength = UNION_ARRAY_LEN;
    armone.Data = ulArray;

    for(i = 0; i < UNION_ARRAY_LEN; i++)
        {
        ulArray[i] = i;
        badunionArray[i].Tag = 1;
        badunionArray[i].u.ulData = i;
        }

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = UnionCall1(Binding, UNION_ARRAY_LEN, badunionArray);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - [in] array of unions.\n", iIterations, mseconds);

    StartTime();
    for(i = iIterations; i; i--)
        {
        status = UnionCall2(Binding, &goodunion);
        if (status != RPC_S_OK)
            {
            goto Cleanup;
            }
        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - [in] union of arrays.\n", iIterations, mseconds);

    }

    // Time pings() (null calls) which impersonate the client.

    StartTime();
    for(i = iIterations; i; i--)
        {

        status = CheckSecurity(Binding);

        if (status != RPC_S_OK)
            {
            if (status == RPC_S_ACCESS_DENIED)
                {
                printf("Access denied, try -s 2 or higher.\n");
                return;
                }
            goto Cleanup;
            }

        }
    mseconds = EndTime();

    printf("%4d calls in %8d milliseconds - void call w/ impersonation\n", iIterations, mseconds);

Cleanup:

    if (status != RPC_S_OK)
        {
        printf("Call failed - %d\n", status);
        }

    return;
}

//
//  FUNCTION: main
//
//  PURPOSE: Parses arguments and binds to the server.
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    Program exit code.
//
//
int main(int argc, char *argv[])
{
    char *serverAddress = NULL;
    char *protocol = "ncalrpc";
    UINT iIterations = 100;
    unsigned char *stringBinding;
    RPC_BINDING_HANDLE Binding;
    RPC_STATUS status;
    ULONG SecurityLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;

    argc--;
    argv++;
    while(argc)
        {
        if (   argv[0][0] != '-'
            && argv[0][0] != '/')
            {
            Usage();
            return(1);
            }

        switch(argv[0][1])
            {
            case 'n':
                if (argc < 2)
                    {
                    Usage();
                    return(1);
                    }
                serverAddress = argv[1];
                argc--;
                argv++;
                break;
            case 't':
                if (argc < 2)
                    {
                    Usage();
                    return(1);
                    }
                protocol = argv[1];
                argc--;
                argv++;
                break;
            case 'i':
                if (argc < 2)
                    {
                    Usage();
                    return(1);
                    }
                iIterations = atoi(argv[1]);
                argc--;
                argv++;
                break;
            case 's':
                if (argc < 2)
                    {
                    Usage();
                    return(1);
                    }
                SecurityLevel = atoi(argv[1]);
                if (SecurityLevel > RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
                    {
                    Usage();
                    return(1);
                    }
                argc--;
                argv++;
                break;
            default:
                Usage();
                return(1);
                break;
            }

        argc--;
        argv++;
        }

    status = RpcStringBindingCompose(0,
                                     protocol,
                                     serverAddress,
                                     0,
                                     0,
                                     &stringBinding);
    if (status != RPC_S_OK)
        {
        printf("RpcStringBindingCompose failed - %d\n", status);
        return(1);
        }

    status = RpcBindingFromStringBinding(stringBinding, &Binding);

    if (status != RPC_S_OK)
        {
        printf("RpcBindingFromStringBinding failed - %d\n", status);
        return(1);
        }

    status =
    RpcBindingSetAuthInfo(Binding,
                          0,
                          SecurityLevel,
                          RPC_C_AUTHN_WINNT,
                          0,
                          0
                         );

    if (status != RPC_S_OK)
        {
        printf("RpcBindingSetAuthInfo failed - %d\n", status);
        return(1);
        }

    status = Ping(Binding);

    if (status != RPC_S_OK)
        {
        printf("Ping failed - %d\n", status);
        }

    printf("Connected.\n");

    //
    // Call and time various RPC calls.
    //

    DoTimings(Binding, iIterations);

    // Cleanup

    status = RpcBindingFree(&Binding);

    // ASSERT(status == RPC_S_OK):

    status = RpcStringFree(&stringBinding);

    // ASSERT(status == RPC_S_OK);

    return(0);
}

void * __RPC_USER MIDL_user_allocate(size_t size)
{
    return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
}

void __RPC_USER MIDL_user_free( void *pointer)
{
    HeapFree(GetProcessHeap(), 0, pointer);
}

