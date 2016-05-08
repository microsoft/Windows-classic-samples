// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: overlap.cpp
//
// Description:
//
//    This module is responsible for handling the overlapped IO 
//    passed to us by the upper layer. If the LSP is on NT, then
//    an IO completion port (IOCP) is created and all overlapped
//    IO initiated by the upper layer passes through our IOCP.
//    If this is Win9x then we create a worker thread and execute
//    the overlapped IO via asynchronous procedure calls (APC).
//
#include "lspdef.h"

#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4127)       // disable "conditional expression is constant" warning
#pragma warning(disable:4702)       // disable "unreacheable code" warning

//
// For each overlapped operation given to us from the upper layer, we'll
// assign a WSAOVERLAPPEDPLUS structure so that we can make the overlapped
// call to the lower layer. The following variables are pointers to these
// structures.
//

static LIST_ENTRY   gPendingOperations;     // list of queued overlapped operations (Win9x)
    
static LIST_ENTRY   gFreeOverlappedPlus;    // look aside list of free structures

//
// NOTE: On Win9x a semaphore is used to trigger when an overlapped operation
//       is queued for execution. On NT based OSes (including XP), a completion
//       port is used. That is, the use of 'gIocp' and 'gWakeupSemaphore' is
//       mutually exclusive.
//

HANDLE             *gWorkerThread = NULL,       // Array of handles to worker threads
                    gWakeupSemaphore = NULL,    // Wakeup semaphore
                    gIocp = NULL;               // Handle to the IOCP
DWORD               gThreadCount = (DWORD)-1;   // Number of worker threads

////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Either allocates a new structure or returns a free one on a lookaside list
LPWSAOVERLAPPEDPLUS 
AllocOverlappedStructure(
    SOCK_INFO  *SocketContext
    );

// Returns the overlapped structure to a list of free structures
void 
FreeOverlappedStructure(
    WSAOVERLAPPEDPLUS  *olp
    );

// Executes an overlapped operation
int 
ExecuteOverlappedOperation(
    WSAOVERLAPPEDPLUS  *lpOverlapped, 
    BOOL                bSynchronous
    );

// On Win9x queues an overlapped operation for later execution. On WinNT, immediately
// execute the call.
int 
EnqueueOverlappedOperation(
    WSAOVERLAPPEDPLUS  *op
    );

// Sets the overlapped in progress flag in the given OVERLAPPED structure
void 
SetOverlappedInProgress(
    OVERLAPPED         *ol
    );

// Overlapped thread which either handles overlapped completion via completion port
// or executes overlapped operations with APCs.
DWORD WINAPI   
OverlappedManagerThread(
    LPVOID              lpParam
    );

// Invokes the APC associated with an overlapped operation to signal completion
VOID CALLBACK 
CallUserApcProc(
    ULONG_PTR           Context
    );

// Checks whether a given socket has been closed in which case all associated resrouces
// can be freed
void 
CheckForContextCleanup(
    WSAOVERLAPPEDPLUS  *ol
    );

////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: InitOverlappedManager
//
// Description:
//    This function is called once and determines whether we're running
//    on NT or Win9x. If on NT, create an IO completion port as the
//    intermediary between the upper and lower layer. All overlapped IO
//    will be posted using this IOCP. When IOCP are available we'll create
//    a number of threads (equal to the number of procs) to service the
//    completion of the IO. If on Win9x, we'll create a single thread which
//    will post overlapped IO operations using APCs.
//
int 
InitOverlappedManager(
    )
{
    DWORD   i;
    int     ret = NO_ERROR;

    EnterCriticalSection( &gOverlappedCS );

    //
    // Make sure we're not already initialized -- we'll always have at least
    // one worker thread running
    //
    if ( NULL != gWorkerThread )
        goto cleanup;

    InitializeListHead( &gFreeOverlappedPlus );
    InitializeListHead( &gPendingOperations );

    //
    // See if we're on NT by trying to create the completion port. If it
    //  fails then we're on Win9x.
    //
    gIocp = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            NULL,
            (ULONG_PTR)0,
            0
            );
    if ( NULL != gIocp )
    {
        SYSTEM_INFO     sinfo;

        //
        // We're on NT so figure out how many processors we have
        //
        GetSystemInfo( &sinfo );
        gThreadCount = sinfo.dwNumberOfProcessors;
    }
    else
    {
        //
        // We're on Win9x so create a semaphore instead. This is used to
        //  wake up the worker thread to service overlapped IO calls.
        //
        gWakeupSemaphore = CreateSemaphore(
                NULL,
                0,
                MAXLONG,
                NULL
                );
        if ( NULL == gWakeupSemaphore )
        {
            dbgprint("InitOverlappedManager: CreateSemaphore() failed: %d", GetLastError());
            ret = WSAEPROVIDERFAILEDINIT;
            goto cleanup;
        }

        //
        // This is Win9x, no multiproc support so create just a single thread
        //
        gThreadCount = 1;
    }

    dbgprint("Creating %d threads", gThreadCount);

    gWorkerThread = (HANDLE *) LspAlloc(
            sizeof( HANDLE ) * gThreadCount,
           &ret         // if fails, ret will be WSAENOBUFS
            );
    if ( NULL == gWorkerThread )
    {
        goto cleanup;
    }

    //
    // Create our worker threads
    //
    for(i=0; i < gThreadCount ;i++)
    {
        gWorkerThread[i] = CreateThread(
                NULL, 
                0, 
                OverlappedManagerThread, 
                (LPVOID)gIocp, 
                0, 
                NULL
                );
        if ( NULL == gWorkerThread[ i ] )
        {
            dbgprint("InitOverlappedManager: CreateThread() failed: %d", GetLastError());
            ret = WSAEPROVIDERFAILEDINIT;
            goto cleanup;
        }
    }

cleanup:

    LeaveCriticalSection( &gOverlappedCS );

    return ret;
}

//
// Function: StopOverlappedManager
//
// Description:
//    This function is called before the DLL gets unloaded. It tries to
//    shutdown the worker threads gracefully before exiting. It also
//    frees/closes allocated memory and handles.
//
int 
StopOverlappedManager(
    )
{
    DWORD     i;
    int       ret = NO_ERROR;

    EnterCriticalSection( &gOverlappedCS );

    //
    // Post a completion packet to the IOCP (one for each thread)
    //
    if ( NULL != gIocp )
    {
        for(i=0; i < gThreadCount ;i++)
        {
            ret = PostQueuedCompletionStatus(
                    gIocp,
                    (DWORD)-1,
                    0,
                    NULL
                    );
            if ( 0 == ret )
            {
                dbgprint("PostQueuedCompletionStatus() failed: %d", GetLastError());
            }
        }

        //
        // Wait a while for the threads to get the signal and exit - if it fails or
        // times out then oh well, we need to clean up anyway
        //
    }
    else
    {
        //
        // On Win9x we closed the semaphore so the worker thread should fail
        // when waiting for a signal and break out of the loop.
        // 

        LIST_ENTRY         *entry = NULL;
        WSAOVERLAPPEDPLUS  *olp = NULL;

        while ( !IsListEmpty( &gPendingOperations ) )
        {
            entry = RemoveHeadList( &gPendingOperations );

            olp = CONTAINING_RECORD( entry, WSAOVERLAPPEDPLUS, Link );

            FreeOverlappedStructure( olp );
        }
    }

    if ( NULL != gWorkerThread )
    {
        ret = WaitForMultipleObjectsEx( gThreadCount, gWorkerThread, TRUE, 5000, TRUE );
        if ( WAIT_TIMEOUT == ret )
            dbgprint("StopOverlappedManager: Timed out waiting for IOCP threads to exit!");
        else if ( WAIT_FAILED == ret )
            dbgprint("StopOverlappedManager: WaitForMultipleObjectsEx failed: %d",
                    GetLastError());
        else
            dbgprint("StopOverlappedManager: All worker threads stopped!");

        for(i=0; i < gThreadCount ;i++)
        {
            CloseHandle( gWorkerThread[ i ] );
            gWorkerThread[ i ] = NULL;

            dbgprint("Closing overlapped thread(s)");
        }

        LspFree( gWorkerThread );
        gWorkerThread = NULL;
    }

    //
    // Cleanup remaining handles...
    //
    if ( NULL != gIocp )
    {
        CloseHandle( gIocp );
        gIocp = NULL;
    }

    if ( NULL != gWakeupSemaphore )
    {
        CloseHandle( gWakeupSemaphore );
        gWakeupSemaphore = NULL;
    }

    LeaveCriticalSection( &gOverlappedCS );

    return ret;
}

//
// Function: QueueOverlappedOperation
//
// Description:
//    Whenever one of the overlapped enabled Winsock SPI functions are
//    called (as an overlapped operation), it calls this function which
//    determines whether it can execute it immediate (in the case of NT
//    and IOCP) or post it to a queue to be executed by the woker thread
//    via an APC (on Win9x).
//
int 
QueueOverlappedOperation(
    WSAOVERLAPPEDPLUS  *ol, 
    SOCK_INFO          *SocketContext
    )
{
    BOOL    bSynchronous = FALSE;
    int     err;

    //
    // Set the fields of the overlapped to indicate IO is not complete yet
    //

    __try
    {
        SetOverlappedInProgress( ol->lpCallerOverlapped );
    } 
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return WSAEFAULT;
    }

    if ( NULL != gIocp )
    {
        //
        // If we haven't already added the provider socket to the IOCP then
        //  do it now.
        //
        AcquireSocketLock( SocketContext );

        if ( NULL == SocketContext->hIocp )
        {
            SocketContext->hIocp = CreateIoCompletionPort(
                    (HANDLE)ol->ProviderSocket,
                    gIocp,
                    ol->CallerSocket,
                    0
                    );
            if ( NULL == SocketContext->hIocp )
            {
                if ( ERROR_INVALID_PARAMETER == (err = GetLastError() ) )
                {
                    //
                    // If the socket option SO_SYNCHRONOUS_(NON)ALERT is set then 
                    // no overlapped operation can be performed on that socket and 
                    // tryiing to associate it with a completion port will fail. 
                    // The other bad news is that an LSP cannot trap this setsockopt 
                    // call. In reality we don't have to do anything. If an app sets 
                    // this option and then makes overlapped calls anyway, then they 
                    // shouldn't be expecting any overlapped notifications! This 
                    // statement is put here in case you want to mark the socket
                    // info structure as synchronous.
                    //
                    bSynchronous = TRUE;
                }
                else
                {
                    dbgprint("QueueOverlappedOperation: CreateIoCompletionPort() "
                              "failed: %d (Prov %d Iocp 0x%x Caller 0x%x 0)", 
                            err, ol->ProviderSocket, 
                            gIocp, ol->CallerSocket);
                }
            }

            dbgprint("Adding provider handle %X to IOCP", ol->ProviderSocket);
        }

        ReleaseSocketLock( SocketContext );

        //
        // Simply execute the operation
        //
        return ExecuteOverlappedOperation( ol, bSynchronous );
    }
    else
    {
        // Queue up the operation for the worker thread to initiate
        //
        return EnqueueOverlappedOperation( ol );
    }
}

//
// Function: EnqueueOverlappedOperation
//
// Description:
//    Enqueue an overlapped operation to be executed by the worker
//    thread via an APC.
//
int 
EnqueueOverlappedOperation(
    WSAOVERLAPPEDPLUS  *op
    )
{
    int ret = WSA_IO_PENDING;

    EnterCriticalSection( &gOverlappedCS );

    if ( NULL == op )
    {
        dbgprint("EnqueueOverlappedOperation: op == NULL!");
        ret = WSAEFAULT;
        goto cleanup;
    }

    InsertTailList( &gPendingOperations, &op->Link );

    //
    // Increment the semaphore count. This lets the worker thread
    // know that there are pending overlapped operations to execute.
    //
    ReleaseSemaphore( gWakeupSemaphore, 1, NULL );

cleanup:

    LeaveCriticalSection( &gOverlappedCS );

    return ret;
}

// 
// Function: DequeueOverlappedOperation
//
// Description:
//    Once the worker thread is notified that there are pending
//    operations, it calls this function to get the first operation
//    pending in order to execute it.
//
WSAOVERLAPPEDPLUS *
DequeueOverlappedOperation(
    )
{
    WSAOVERLAPPEDPLUS   *op = NULL;
    LIST_ENTRY         *link = NULL;

    EnterCriticalSection( &gOverlappedCS );

    link = RemoveHeadList( &gPendingOperations );

    op = CONTAINING_RECORD( link, WSAOVERLAPPEDPLUS, Link );

    /*
    op = gPendingStart;
    if ( NULL != gPendingStart )
    {
        gPendingStart = op->next;
    }
    if (op == gPendingEnd)
    {
        gPendingStart = gPendingEnd = NULL;
    }
    */

    LeaveCriticalSection( &gOverlappedCS );

    return op;
}

//
// Function: ExecuteOverlappedOperation
//
// Description:
//    This function actually executes an overlapped operation that was queued.
//    If on Win9x we substitute our own completion function in order to intercept
//    the results. If on NT we post the operation to our completion port.
//    This function either returns NO_ERROR if the operation succeeds immediately
//    or the Winsock error code upone failure (or overlapped operation).
//
int 
ExecuteOverlappedOperation(
    WSAOVERLAPPEDPLUS *ol, 
    BOOL bSynchronous
    )
{
    LPWSAOVERLAPPED_COMPLETION_ROUTINE   routine = NULL;
    PROVIDER                            *Provider;
    DWORD                               *lpdwFlags = NULL,
                                        *lpdwBytes = NULL,
                                         dwBytes;
    int                                  ret=SOCKET_ERROR, err=0;

    if ( NULL == gIocp)
        routine = IntermediateCompletionRoutine;

    Provider = ol->Provider;

    //
    // Reset the event handle if present. The handle is masked with 0xFFFFFFFE in
    //  order to zero out the last bit. If the last bit is one and the socket is
    //  associated with a compeltion port then when an overlapped operation is 
    //  called, the operation is not posted to the IO completion port.
    //
    if ( NULL != ol->lpCallerOverlapped->hEvent )
    {
        ULONG_PTR   ptr = 1;

        ResetEvent( (HANDLE) ( (ULONG_PTR) ol->lpCallerOverlapped->hEvent & ~ptr ) );
    }

    switch ( ol->Operation )
    {
        case LSP_OP_IOCTL:
            lpdwFlags = NULL;
            lpdwBytes = &ol->IoctlArgs.cbBytesReturned;
            ret = Provider->NextProcTable.lpWSPIoctl(
                    ol->ProviderSocket,
                    ol->IoctlArgs.dwIoControlCode,
                    ol->IoctlArgs.lpvInBuffer,
                    ol->IoctlArgs.cbInBuffer,
                    ol->IoctlArgs.lpvOutBuffer,
                    ol->IoctlArgs.cbOutBuffer,
                   &ol->IoctlArgs.cbBytesReturned,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                   );
            break;                         

        case LSP_OP_RECV:
            lpdwFlags = &ol->RecvArgs.dwFlags;
            lpdwBytes = &ol->RecvArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTable.lpWSPRecv(
                    ol->ProviderSocket,
                    ol->RecvArgs.lpBuffers,
                    ol->RecvArgs.dwBufferCount,
                   &ol->RecvArgs.dwNumberOfBytesRecvd,
                   &ol->RecvArgs.dwFlags,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_RECVFROM:
            lpdwFlags = &ol->RecvFromArgs.dwFlags;
            lpdwBytes = &ol->RecvFromArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTable.lpWSPRecvFrom(
                    ol->ProviderSocket,
                    ol->RecvFromArgs.lpBuffers,
                    ol->RecvFromArgs.dwBufferCount,
                   &ol->RecvFromArgs.dwNumberOfBytesRecvd,
                   &ol->RecvFromArgs.dwFlags,
                    ol->RecvFromArgs.lpFrom,
                    ol->RecvFromArgs.lpFromLen,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_SEND:
            lpdwFlags = &ol->SendArgs.dwFlags;
            lpdwBytes = &ol->SendArgs.dwNumberOfBytesSent;
            ret = Provider->NextProcTable.lpWSPSend(
                    ol->ProviderSocket,
                    ol->SendArgs.lpBuffers,
                    ol->SendArgs.dwBufferCount,
                   &ol->SendArgs.dwNumberOfBytesSent,
                    ol->SendArgs.dwFlags,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                   );
             break;

        case LSP_OP_SENDTO:
            lpdwFlags = &ol->SendToArgs.dwFlags;
            lpdwBytes = &ol->SendToArgs.dwNumberOfBytesSent;
            ret = Provider->NextProcTable.lpWSPSendTo(
                    ol->ProviderSocket,
                    ol->SendToArgs.lpBuffers,
                    ol->SendToArgs.dwBufferCount,
                   &ol->SendToArgs.dwNumberOfBytesSent,
                    ol->SendToArgs.dwFlags,
                    (SOCKADDR *)&ol->SendToArgs.To,
                    ol->SendToArgs.iToLen,
                   &ol->ProviderOverlapped,
                    routine,
                   &ol->CallerThreadId,
                   &err
                    );
            break;

        case LSP_OP_TRANSMITFILE:
            lpdwFlags = &ol->TransmitFileArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnTransmitFile(
                    ol->ProviderSocket,
                    ol->TransmitFileArgs.hFile,
                    ol->TransmitFileArgs.nNumberOfBytesToWrite,
                    ol->TransmitFileArgs.nNumberOfBytesPerSend,
                   &ol->ProviderOverlapped,
                    ol->TransmitFileArgs.lpTransmitBuffers,
                    ol->TransmitFileArgs.dwFlags
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_ACCEPTEX:
            lpdwFlags = NULL;
            lpdwBytes = &ol->AcceptExArgs.dwBytesReceived;
            ret = Provider->NextProcTableExt.lpfnAcceptEx(
                    ol->ProviderSocket,
                    ol->AcceptExArgs.sProviderAcceptSocket,
                    ol->AcceptExArgs.lpOutputBuffer,
                    ol->AcceptExArgs.dwReceiveDataLength,
                    ol->AcceptExArgs.dwLocalAddressLength,
                    ol->AcceptExArgs.dwRemoteAddressLength,
                   &ol->AcceptExArgs.dwBytesReceived,
                   &ol->ProviderOverlapped
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_CONNECTEX:
            lpdwFlags = NULL;
            lpdwBytes = &ol->ConnectExArgs.dwBytesSent;
            ret = Provider->NextProcTableExt.lpfnConnectEx(
                    ol->ProviderSocket,
                    (SOCKADDR *)&ol->ConnectExArgs.name,
                    ol->ConnectExArgs.namelen,
                    ol->ConnectExArgs.lpSendBuffer,
                    ol->ConnectExArgs.dwSendDataLength,
                   &ol->ConnectExArgs.dwBytesSent,
                   &ol->ProviderOverlapped
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_DISCONNECTEX:
            lpdwFlags = &ol->DisconnectExArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnDisconnectEx(
                    ol->ProviderSocket,
                   &ol->ProviderOverlapped,
                    ol->DisconnectExArgs.dwFlags,
                    ol->DisconnectExArgs.dwReserved
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_TRANSMITPACKETS:
            lpdwFlags = &ol->TransmitPacketsArgs.dwFlags;
            lpdwBytes = NULL;
            ret = Provider->NextProcTableExt.lpfnTransmitPackets(
                    ol->ProviderSocket,
                    ol->TransmitPacketsArgs.lpPacketArray,
                    ol->TransmitPacketsArgs.nElementCount,
                    ol->TransmitPacketsArgs.nSendSize,
                   &ol->ProviderOverlapped,
                    ol->TransmitPacketsArgs.dwFlags
                    );
            if ( FALSE == ret )
            {
                ret = SOCKET_ERROR;
                err = WSAGetLastError();
                WSASetLastError(err);
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_RECVMSG:
            lpdwFlags = NULL;
            lpdwBytes = &ol->RecvMsgArgs.dwNumberOfBytesRecvd;
            ret = Provider->NextProcTableExt.lpfnWSARecvMsg(
                    ol->ProviderSocket,
                   &ol->RecvMsgArgs.WsaMsg,
                   &ol->RecvMsgArgs.dwNumberOfBytesRecvd,
                   &ol->ProviderOverlapped,
                    routine
                    );
            if ( SOCKET_ERROR == ret )
            {
                err = WSAGetLastError();
                WSASetLastError( err );
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        case LSP_OP_SENDMSG:

            lpdwFlags = &ol->SendMsgArgs.SendMsg.dwFlags;
            lpdwBytes =  ol->SendMsgArgs.SendMsg.lpNumberOfBytesSent;

            ret = Provider->NextProcTable.lpWSPIoctl(
                    ol->ProviderSocket,
                    SIO_EXT_SENDMSG,
                   &ol->SendMsgArgs.SendMsg,
                    sizeof(ol->SendMsgArgs.SendMsg),
                   &err,
                    sizeof(err),
                   &dwBytes,
                    NULL,
                    NULL,
                   &ol->CallerThreadId,
                   &err
                    );
            if (NO_ERROR != err)
            {
                ret = SOCKET_ERROR;
            }
            else
            {
                ret = NO_ERROR;
            }
            break;

        default:
            dbgprint("ExecuteOverlappedOperation: Unknown operation!");
            ret = SOCKET_ERROR;
            break;
    }

    if ( ( NO_ERROR != ret ) && ( WSA_IO_PENDING != err ) )
    {
        //
        // If the call immediately fails, update the OVERLAPPED info and return
        //
        ol->lpCallerOverlapped->Offset       = (lpdwFlags ? *lpdwFlags : 0);
        ol->lpCallerOverlapped->OffsetHigh   = err;
        ol->lpCallerOverlapped->InternalHigh = (lpdwBytes ? *lpdwBytes : 0);
        ol->lpCallerOverlapped->Internal     = 0;

        dbgprint("Overlap op failed immediately: %d", ol->Error);

        CheckForContextCleanup( ol );

        FreeOverlappedStructure( ol );
    }
    else if ( ( NO_ERROR == ret ) && ( FALSE == bSynchronous ) )
    {
        // 
        // NOTE: We could return success from here, but if you want to perform
        //       some post processing on the data buffers before indicating
        //       success to the upper layer (because once success is returned, the
        //       upper layer can inspect the buffers). Because of this we return
        //       pending and in the completion processing (either via IOCP or
        //       APC), we can do post processing there.
        //
        err = WSA_IO_PENDING;
        ret = SOCKET_ERROR;
    }
    else if ( ( NO_ERROR == ret ) && ( TRUE == bSynchronous ) )
    {
        // The winsock call actually blocked and there will be no completion
        // notification on the IOCP.
        //
        dbgprint("Succeeded without error - synchronous socket though");
    }

    return ( ( NO_ERROR == ret ) ? ret : err );
}

//
// Function: OverlappedManagerThread
//
// Description:
//    This thread receives the completion notifications for operations
//    posted to our IO completion port. Once we receive a completion
//    we'll complete the operation back to the calling app.
//
DWORD WINAPI 
OverlappedManagerThread(
    LPVOID lpParam
    )
{
    WSAOVERLAPPEDPLUS *pOverlapPlus = NULL;
    WSAOVERLAPPED     *pOverlap = NULL;
    HANDLE             hIocp = (HANDLE)lpParam;
    ULONG_PTR          key;
    DWORD              dwBytesXfered;
    int                ret;

    while ( TRUE )
    {
        if ( NULL != hIocp )
        {
            ret = GetQueuedCompletionStatus(
                    hIocp,
                   &dwBytesXfered,
                   &key,
                   &pOverlap,
                    INFINITE
                    );
            if ( 0 == ret )
            {
                // Socket failures could be reported here so we still
                // call IntermediateCompletionRoutine
                dbgprint("GetQueuedCompletionStatus() failed: %d", GetLastError());
            }

            if ( -1 == dwBytesXfered )
            {
                //
                // StopOverlappedManager will send a completion packet with -1
                //    bytes transfered to indicate the completion routine
                //    should exit
                //
                dbgprint("OverlappedManagerThread: Received exit message");
                goto cleanup;
            }

            // Handle the IO that completed
            IntermediateCompletionRoutine(
                    WSA_IO_PENDING,
                    dwBytesXfered,
                    pOverlap,
                    0
                    );
        }
        else
        {
            ret = WaitForSingleObjectEx(
                    gWakeupSemaphore,
                    INFINITE, 
                    TRUE
                    );
            if ( WAIT_IO_COMPLETION == ret )
            {
                // An APC fired so keep waiting until semaphore is signaled
                continue;
            }
            else if ( WAIT_OBJECT_0 == ret )
            {
                pOverlapPlus = DequeueOverlappedOperation();
                if ( NULL == pOverlapPlus )
                    continue;

                ExecuteOverlappedOperation( pOverlapPlus, FALSE );
            }
            else
            {
                dbgprint("OverlappedManagerThread: WaitForSingleObjectEx() failed: %d (error = %d)",
                        ret, GetLastError() );
                goto cleanup;
            }
        }
    }

cleanup:

    ExitThread( 0 );
}

//
// Function: AllocOverlappedStructure
//
// Description:
//    This returns an unused WSAOVERLAPPEDPLUS structure. We maintain a list
//    of freed structures so that we don't spend too much time allocating and
//    freeing memory.
//
LPWSAOVERLAPPEDPLUS 
AllocOverlappedStructure(
    SOCK_INFO  *SocketContext
    )
{
    LPWSAOVERLAPPEDPLUS lpWorkerOverlappedPlus = NULL;

    if ( NULL == SocketContext )
    {
        dbgprint("AllocOverlappedStructure: SocketContext is NULL!");
        return NULL;
    }

    EnterCriticalSection( &gOverlappedCS );

    AcquireSocketLock( SocketContext );

    //
    // We have to keep track of the number of outstanding overlapped requests 
    // an application has. Otherwise, if the app were to close a socket that 
    // had oustanding overlapped ops remaining, we'd start leaking structures 
    // in gOverlappedPool. The idea here is to force the CallerSocket to remain 
    // open until the lower provider has processed all the overlapped requests. 
    // If we closed both the lower socket and the caller socket, we would no 
    // longer be able to correlate completed requests to any apps sockets.
    //
    (SocketContext->dwOutstandingAsync)++;

    if ( IsListEmpty( &gFreeOverlappedPlus ) )
    {
        int     err;

        lpWorkerOverlappedPlus = (WSAOVERLAPPEDPLUS *) LspAlloc( 
                sizeof(WSAOVERLAPPEDPLUS), 
               &err 
                );
    }
    else
    {
        LIST_ENTRY  *entry = NULL;

        entry = RemoveHeadList( &gFreeOverlappedPlus );

        lpWorkerOverlappedPlus = CONTAINING_RECORD( entry, WSAOVERLAPPEDPLUS, Link );
    }

    ReleaseSocketLock( SocketContext );

    LeaveCriticalSection( &gOverlappedCS );

    return lpWorkerOverlappedPlus;
}

//
// Function: FreeOverlappedStructure
//
// Description:
//    Once we're done using a WSAOVERLAPPEDPLUS structure we return it to
//    a list of free structures for re-use later.
//
void 
FreeOverlappedStructure(
    WSAOVERLAPPEDPLUS  *olp
    )
{
    EnterCriticalSection( &gOverlappedCS );

    switch ( olp->Operation )
    {
        case LSP_OP_RECV:
            FreeWSABuf( olp->RecvArgs.lpBuffers );
            break;

        case LSP_OP_RECVFROM:
            FreeWSABuf( olp->RecvFromArgs.lpBuffers );
            break;

        case LSP_OP_SEND:
            FreeWSABuf( olp->SendArgs.lpBuffers );
            break;

        case LSP_OP_SENDTO:
            FreeWSABuf( olp->SendToArgs.lpBuffers );
            break;

        default:
            break;
    }

    memset( olp, 0, sizeof( WSAOVERLAPPEDPLUS ) );

    InsertHeadList( &gFreeOverlappedPlus, &olp->Link );

    LeaveCriticalSection( &gOverlappedCS );
}

//
// Function: SetOverlappedInProgress
//
// Description:
//    Simply set the interal fields of an OVERLAPPED structure to the
//    "in progress" state.
//
void 
SetOverlappedInProgress(
    OVERLAPPED *ol
    )
{
    ol->Internal = WSS_OPERATION_IN_PROGRESS;
    ol->InternalHigh = 0;
}

//
// Function: IntermediateCompletionRoutine
//
// Description:
//    Once an overlapped operation completes we call this routine to keep
//    count of the bytes sent/received as well as to complete the operation
//    to the application.
//
void CALLBACK 
IntermediateCompletionRoutine(
    DWORD dwError,
    DWORD cbTransferred,
    LPWSAOVERLAPPED lpOverlapped,
    DWORD dwFlags
    )
{
    LPWSAOVERLAPPEDPLUS olp = NULL;
    SOCK_INFO          *SocketContext = NULL,
                       *AcceptSocketContext = NULL;
    int                 Error,
                        ret;

    if ( NULL == lpOverlapped )
    {
        dbgprint("IntermediateCompletionRoutine: lpOverlapped == NULL!");
        goto cleanup;
    }

    ASSERT( lpOverlapped );

    olp = CONTAINING_RECORD( lpOverlapped, WSAOVERLAPPEDPLUS, ProviderOverlapped );

    //
    // We actually already have the socket context for this operation (its in
    //    the WSAOVERLAPPEDPLUS structure but do this anyway to make sure the
    //    socket hasn't been closed as well as to increment the ref count while
    //    we're accessing the SOCK_INFO structure.
    //
    SocketContext = FindAndRefSocketContext(olp->CallerSocket, &Error);
    if ( NULL == SocketContext )
    {
        dbgprint( "IntermediateCompletionRoutine: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    if ( WSA_IO_PENDING == dwError )
    {
        //
        // Get the results of the operation
        //

        ASSERT( olp->Provider );
        ASSERT( olp->Provider->NextProcTable.lpWSPGetOverlappedResult );

        dwError = NO_ERROR;
        ret = olp->Provider->NextProcTable.lpWSPGetOverlappedResult(
                olp->ProviderSocket,
                lpOverlapped,
               &cbTransferred,
                FALSE,
               &dwFlags,
                (int *)&dwError
                );
 
        if ( FALSE == ret )
        {
            dbgprint("IntermediateCompletionRoutine: WSPGetOverlappedResult failed: %d", dwError);
        }
        else
        {

            dbgprint("Bytes transferred on socket 0x%x: %d [op=%d; err=%d]", 
                    olp->CallerSocket, cbTransferred, olp->Operation, dwError);
        }
    }

    olp->lpCallerOverlapped->Offset       = dwFlags;
    olp->lpCallerOverlapped->OffsetHigh   = dwError;
    olp->lpCallerOverlapped->InternalHigh = cbTransferred;

    SocketContext->LastError = dwError;

    if ( 0 == dwError )
    {
        AcquireSocketLock( SocketContext );

        //
        // NOTE: This is where any post processing should go for overlapped operations.
        //       For example, if you wanted to inspect the data received, you would do
        //       that here for any operation that receives data (don't forget AcceptEx!).
        //       In this sample, all we do is count the bytes sent and received on the
        //       socket.
        //

        switch ( olp->Operation )
        {
            case LSP_OP_RECV:
                SocketContext->BytesRecv += cbTransferred;
                FreeWSABuf(olp->RecvArgs.lpBuffers);
                break;

            case LSP_OP_RECVFROM:
                SocketContext->BytesRecv += cbTransferred;
                FreeWSABuf(olp->RecvFromArgs.lpBuffers);
                break;

            case LSP_OP_SEND:
                SocketContext->BytesSent += cbTransferred;
                FreeWSABuf(olp->SendArgs.lpBuffers);
                break;

            case LSP_OP_SENDTO:
                SocketContext->BytesSent += cbTransferred;
                FreeWSABuf(olp->SendToArgs.lpBuffers);
                break;

            case LSP_OP_TRANSMITFILE:
                SocketContext->BytesSent += cbTransferred;
                break;

            case LSP_OP_TRANSMITPACKETS:
                SocketContext->BytesSent += cbTransferred;
                break;

            case LSP_OP_ACCEPTEX:

                ReleaseSocketLock( SocketContext );

                AcceptSocketContext = FindAndRefSocketContext(
                        olp->AcceptExArgs.sAcceptSocket,
                       &Error
                       );
                if (AcceptSocketContext == NULL)
                {
                    dbgprint( "IntermediateCompletionRoutine: FindAndRefSocketContext failed! (LSP_OP_ACCEPTEX)" );
                    dwError = WSAENOTSOCK;
                    olp->lpCallerOverlapped->OffsetHigh = dwError;
                }
                else
                {
                    AcquireSocketLock( AcceptSocketContext );

                    // Release the reference on the accepted socket object
                    AcceptSocketContext->dwOutstandingAsync--;
                    AcceptSocketContext->BytesRecv += cbTransferred;

                    ReleaseSocketLock( AcceptSocketContext );

                    DerefSocketContext(AcceptSocketContext, &Error);
                }

                break;

            case LSP_OP_CONNECTEX:
                SocketContext->BytesSent += cbTransferred;
                break;

            case LSP_OP_RECVMSG:
                SocketContext->BytesRecv += cbTransferred;
                FreeWSABuf(olp->RecvMsgArgs.WsaMsg.lpBuffers);
                break;

            case LSP_OP_SENDMSG:
                SocketContext->BytesSent += cbTransferred;
                FreeWSABuf(olp->SendMsgArgs.SendMsg.lpMsg->lpBuffers);
                break;

            default:
                break;
        }
        // Already released for AcceptEx operations
        if ( LSP_OP_ACCEPTEX != olp->Operation )
            ReleaseSocketLock( SocketContext );
    }

    DerefSocketContext( SocketContext, &Error );

    if ( NULL != olp->lpCallerCompletionRoutine )
    {
        //
        // If the app supplied a completion routine, queue it up for completion
        //
        olp->lpCallerOverlapped->Internal = (ULONG_PTR)olp->lpCallerCompletionRoutine;

        if ( olp->lpCallerOverlapped->Internal == (ULONG_PTR)0x103)
            DebugBreak();

        ret = gMainUpCallTable.lpWPUQueueApc(
               &olp->CallerThreadId,
               CallUserApcProc,
               (DWORD_PTR) olp->lpCallerOverlapped,
              &Error
               );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("IntermediateCompletionRoutine: WPUQueueApc() failed: %d", Error);
        }
    }
    else
    {
        //
        // Otherwise we signal that the op has completed
        //
        ret = WPUCompleteOverlappedRequest(
                olp->CallerSocket,
                olp->lpCallerOverlapped,
                dwError,
                cbTransferred,
               &Error
                );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WPUCompleteOverlappedRequest failed: %d (provider socket 0x%x)", 
                    Error, olp->CallerSocket );
        }
    }

    if ( ( NULL != olp ) && ( TRUE == olp->CloseThread ) )
    {
        ret = gMainUpCallTable.lpWPUCloseThread( &olp->CallerThreadId, &Error );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WPUCloseThread failed: %d", Error );
        }
        olp->CloseThread = FALSE;
    }

    //
    // Cleanup the accounting on the socket
    //
    CheckForContextCleanup( olp );

cleanup:

    if ( NULL != olp )
        FreeOverlappedStructure( olp );

    return;
}

//
// Function: CallerUserApcProc
//
// Description:
//    This function completes an overlapped request that supplied an APC.
//
VOID CALLBACK 
CallUserApcProc(
    ULONG_PTR   Context
    )
{
    LPOVERLAPPED                        lpOverlapped;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE  UserCompletionRoutine;

    lpOverlapped = (LPOVERLAPPED) Context;
    UserCompletionRoutine  = (LPWSAOVERLAPPED_COMPLETION_ROUTINE)lpOverlapped->Internal;
    lpOverlapped->Internal = 0; // Remove the WSS_OPERATION_IN_PROGRESS value 

    UserCompletionRoutine(
            (DWORD)lpOverlapped->OffsetHigh,
            (DWORD)lpOverlapped->InternalHigh,
            lpOverlapped,
            (DWORD)lpOverlapped->Offset
            );
    return;
}

//
// Function: CheckForContextCleanup
//
// Description:
//    After an overlapped operation completes (or from WSPCloseSocket) we
//    need to see if the socket has been marked for closure and make sure
//    no one else is referencing it. If so then the associated structures
//    can be freed and the socket closed.
//
void 
CheckForContextCleanup(
    WSAOVERLAPPEDPLUS  *ol
    )
{
    SOCK_INFO  *SocketContext = NULL;
    int         Error,
                ret;

    SocketContext = FindAndRefSocketContext( ol->CallerSocket, &Error );
    if ( NULL == SocketContext )
    {
        dbgprint( "CheckForContextCleanup: FindAndRefSocketContext failed!" );
        return;
    }

    EnterCriticalSection( &gCriticalSection );

    AcquireSocketLock( ol->SockInfo );

    (ol->SockInfo->dwOutstandingAsync)--;

    if ( ( TRUE == ol->SockInfo->bClosing ) && 
         ( 0 == ol->SockInfo->dwOutstandingAsync ) &&
         ( 1 == ol->SockInfo->RefCount )
       )
    {
        //
        // If the calling app closed the socket while there were still outstanding
        //  async operations then all the outstanding operations have completed so
        //  we can close the apps socket handle.
        //
        ret = gMainUpCallTable.lpWPUCloseSocketHandle(
                ol->CallerSocket, 
               &ol->Error
                );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("CheckForContextClenaup: WPUCloseSocketHandle() failed: %d", ol->Error);
        }

        ol->SockInfo->LayeredSocket = INVALID_SOCKET;

        RemoveSocketInfo( ol->SockInfo->Provider, ol->SockInfo );

        dbgprint("CheckForContxtCleanup: Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]", 
                ol->CallerSocket, ol->SockInfo->BytesSent, ol->SockInfo->BytesRecv);

        ReleaseSocketLock( ol->SockInfo );

        ol->SockInfo = NULL;

        FreeSockInfo( SocketContext );

        goto cleanup;
    }

    ReleaseSocketLock( ol->SockInfo );

    DerefSocketContext( SocketContext, &Error );

cleanup:

    LeaveCriticalSection( &gCriticalSection );

    return;
}

//
// Function: PrepareOverlappedOperation
//
// Description:
//    This function simply allocates and initializes the common fields of a
//    WSAOVERLAPPEDPLUS structure. These fields are required in order to handle
//    overlapped IO correctly.
//
WSAOVERLAPPEDPLUS *
PrepareOverlappedOperation(
    SOCK_INFO                         *SocketContext,
    LspOperation                       operation,
    WSABUF                            *lpBuffers,
    DWORD                              dwBufferCount,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID                      lpThreadId,
    int                               *lpErrno
    )
{
    WSAOVERLAPPEDPLUS *ProviderOverlapped = NULL;
    int                ret = SOCKET_ERROR,
                       err = 0;

    // Allocate a WSAOVERLAPPEDPLUS structure
    ProviderOverlapped = AllocOverlappedStructure( SocketContext );
    if ( NULL == ProviderOverlapped )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }

    __try 
    {
        // Check for an event and reset if. Also, copy the offsets from the upper
        // layer's WSAOVERLAPPED structure.
        if ( ( NULL == lpCompletionRoutine ) && 
             ( NULL != lpOverlapped->hEvent ) )
        { 
            ULONG_PTR   ptr = 1;

            ret = ResetEvent( (HANDLE) ( (ULONG_PTR) lpOverlapped->hEvent & ~ptr ) );
            if (ret == 0)
            {
                *lpErrno = ERROR_INVALID_HANDLE;
                ret = SOCKET_ERROR;
                goto cleanup;
            }
        }

        // Copy any offset information from the caller's overlapped to ours
        CopyOffset( &ProviderOverlapped->ProviderOverlapped, lpOverlapped );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        *lpErrno = WSAEFAULT;
        goto cleanup;
    }

    if ( NULL != lpThreadId )
    {
        ProviderOverlapped->CallerThreadId = *lpThreadId;
    }
    else
    {
        // If thread info wasn't passed to us, we need to open the thread context
        ret = gMainUpCallTable.lpWPUOpenCurrentThread( &ProviderOverlapped->CallerThreadId, &err );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WPUOpenCurrentThread failed: %d", err);
        }
        else
        {
            // Need to remember for later to close the context since we opened it
            ProviderOverlapped->CloseThread = TRUE;
        }
    }

    // Fill in the remaining fields
    ProviderOverlapped->Provider           = SocketContext->Provider;
    ProviderOverlapped->lpCallerOverlapped = lpOverlapped;
    ProviderOverlapped->lpCallerCompletionRoutine = lpCompletionRoutine;
    ProviderOverlapped->SockInfo           = SocketContext;
    ProviderOverlapped->CallerSocket       = SocketContext->LayeredSocket;
    ProviderOverlapped->ProviderSocket     = SocketContext->ProviderSocket;
    ProviderOverlapped->Error              = NO_ERROR;
    ProviderOverlapped->Operation          = operation;

    if ( ( NULL != lpBuffers) && ( dwBufferCount ) )
    {
        // Depending on the underlying operation, copy some parameters specific to it
        switch ( operation )
        {
            case LSP_OP_RECV:
                ProviderOverlapped->RecvArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->RecvArgs.lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if ( NULL == ProviderOverlapped->RecvArgs.lpBuffers )
                    goto cleanup;

                break;

            case LSP_OP_RECVFROM:
                ProviderOverlapped->RecvFromArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->RecvFromArgs.lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if ( NULL == ProviderOverlapped->RecvFromArgs.lpBuffers )
                    goto cleanup;

                break;

            case LSP_OP_SEND:
                ProviderOverlapped->SendArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->SendArgs.lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if ( NULL == ProviderOverlapped->SendArgs.lpBuffers )
                    goto cleanup;

                break;
              
            case LSP_OP_SENDTO:
                ProviderOverlapped->SendToArgs.dwBufferCount = dwBufferCount;
                ProviderOverlapped->SendToArgs.lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if ( NULL == ProviderOverlapped->SendToArgs.lpBuffers )
                    goto cleanup;

                break;

            case LSP_OP_RECVMSG:
                ProviderOverlapped->RecvMsgArgs.WsaMsg.dwBufferCount = dwBufferCount;
                ProviderOverlapped->RecvMsgArgs.WsaMsg.lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if (NULL == ProviderOverlapped->RecvMsgArgs.WsaMsg.lpBuffers)
                    goto cleanup;
                break;

            case LSP_OP_SENDMSG:

                ProviderOverlapped->SendMsgArgs.SendMsg.lpMsg = 
                    &ProviderOverlapped->SendMsgArgs.WsaMsg;
                ProviderOverlapped->SendMsgArgs.SendMsg.lpMsg->dwBufferCount = dwBufferCount;
                ProviderOverlapped->SendMsgArgs.SendMsg.lpOverlapped = &ProviderOverlapped->ProviderOverlapped;
                ProviderOverlapped->SendMsgArgs.SendMsg.lpMsg->lpBuffers = CopyWSABuf(
                        lpBuffers,
                        dwBufferCount,
                        lpErrno
                        );
                if (NULL == ProviderOverlapped->SendMsgArgs.SendMsg.lpMsg->lpBuffers)
                    goto cleanup;

                break;


            default:
                break;
        }
    }

    ret = NO_ERROR;

cleanup:

    if ( SOCKET_ERROR == ret )
    {
        UndoOverlappedOperation( SocketContext, ProviderOverlapped );
        ProviderOverlapped = NULL;
    }
    else
    {
        ASSERT( NO_ERROR == ret );
    }

    return ProviderOverlapped;
}

//
// Function: UndoOverlappedOperation
//
// Description:
//    This function decrements the outstanding asynchronous count as well as
//    freeing any resources associated with this structure (such as the copied
//    WSABUF array for those asynchronous calls that use them).
//
void
UndoOverlappedOperation( 
    SOCK_INFO         *SocketContext,
    WSAOVERLAPPEDPLUS *ProviderOverlapped
    )
{
    AcquireSocketLock( SocketContext );
    (SocketContext->dwOutstandingAsync)--;
    ReleaseSocketLock( SocketContext );

    FreeOverlappedStructure( ProviderOverlapped );
}

//
// Function: FreeOverlappedLookasideList
// 
// Description:
//      This routine frees all WSAOVELRAPPEDPLUS structures in the lookaside list.
//      This function is called when the LSP is unloading.
//
void
FreeOverlappedLookasideList(
    )
{
    WSAOVERLAPPEDPLUS  *olp = NULL;
    LIST_ENTRY         *entry = NULL;

    EnterCriticalSection( &gOverlappedCS );

    while ( ! IsListEmpty( &gFreeOverlappedPlus ) )
    {
        entry = RemoveHeadList( &gFreeOverlappedPlus );

        olp = CONTAINING_RECORD( entry, WSAOVERLAPPEDPLUS, Link );

        LspFree( olp );
    }

    LeaveCriticalSection( &gOverlappedCS );
}
