// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: sockinfo.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains routines associated with the SOCK_INFO structure. This
//    structure maintains the mapping between the upper layer's socket and the
//    corresponding lower layer's socket. It also keeps track of the current
//    state and what operations are pending on the socket. The routines in this
//    file are for allocating, linked list management, etc.
//    

#include "lspdef.h"

// Lookup a socket context structure given the lower provider's socket
SOCK_INFO *
FindSockInfoFromProviderSocket(
    PROVIDER   *provider,
    SOCKET      socket
    );

//
// Function: FindAndRefSocketContext
//
// Description:
//    This routine grabs the LSP critical seciton to lookup the socket context
//    and increase its ref count. Any operation on the socket context holds
//    the critical section so that it cannot be freed while its state changes.
//
SOCK_INFO *
FindAndRefSocketContext(
    SOCKET  s, 
    int    *lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    int        ret;

    EnterCriticalSection(&gCriticalSection);

    ASSERT( gMainUpCallTable.lpWPUQuerySocketHandleContext );

    ret = gMainUpCallTable.lpWPUQuerySocketHandleContext(
            s,
            (PDWORD_PTR) &SocketContext,
            lpErrno
            );
    if ( SOCKET_ERROR == ret )
    {
        dbgprint("FindAndRefSocketContext: WPUQuerySocketHandleContext failed: %d", *lpErrno);
        *lpErrno = WSAENOTSOCK;
    }
    else
    {
        InterlockedIncrement(&SocketContext->RefCount);
    }

    LeaveCriticalSection(&gCriticalSection);

    return SocketContext;
}

//
// Function: DerefSocketContext
//
// Description:
//    This routine holds the LSP critical section and decrements the ref count
//    by one. It also checks if the socket has been closed while holding the
//    ref count. This can happen if two threads are accessing a socket simultaneously
//    and one calls closesocket. We don't want to remove the context from under
//    the second thread so it is marked as closing instead.
//
void 
DerefSocketContext(
    SOCK_INFO  *context, 
    int        *lpErrno
    )
{
    LONG    newval;
    int     ret = NO_ERROR;

    EnterCriticalSection(&gCriticalSection);

    // Decrement the ref count and see if someone closed this socket (from another thread)
    newval = InterlockedDecrement(&context->RefCount);
    if ( ( 0 == newval ) && 
         ( 0 == context->dwOutstandingAsync ) && 
         ( TRUE == context->bClosing ) 
       )
    {
        ASSERT( gMainUpCallTable.lpWPUCloseSocketHandle );

        // Socket has been closed so close the handle and free associated resources
        ret = gMainUpCallTable.lpWPUCloseSocketHandle(context->LayeredSocket, lpErrno);
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("DerefSocketContext: WPUCloseSocketHandle() failed: %d", *lpErrno);
        }

        context->LayeredSocket = INVALID_SOCKET;

        RemoveSocketInfo(context->Provider, context);

        dbgprint("Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]", 
                context->LayeredSocket, context->BytesSent, context->BytesRecv);

        FreeSockInfo( context );
        context = NULL;
    }

    LeaveCriticalSection( &gCriticalSection );
}

//
// Function: AcquireSocketLock
// 
// Description:
//    This routine acquires the critical section which is a member of the 
//    socket's context structure. This is held when modifying the socket
//    context outside of looking up the context (which is performed by
//    FindAndRefSocketContext).
//
void 
AcquireSocketLock(
    SOCK_INFO  *SockInfo
    )
{
    EnterCriticalSection( &SockInfo->SockCritSec );
}

//
// Function: ReleaseSocketLock
//
// Description:
//    This routine releases the socket context critical section.
//
void 
ReleaseSocketLock(
    SOCK_INFO  *SockInfo
    )
{
    LeaveCriticalSection( &SockInfo->SockCritSec );
}

//
// Function: CreateSockInfo
//
// Description:
//    Allocates a new socket info context structure and initializes the fields
//    except for the LayeredSocket field. The context must be allocated first,
//    then the layered socket is created (with the SOCK_INFO structure as the
//    context information), and then the LayeredSocket field is set. If
//    the Inherit context is provided, information is copied to the new socket
//    context structure (such as with WSPAccept). If the Insert flag is TRUE
//    then the context is automatically inserted into the list of sockets
//    for the given provider. If not then the caller must insert the context
//    (WSPAccept does this to ensure all fields of the context are valid
//    including LayeredSocket before insertion so that the async thread
//    handler will work properly).
//
SOCK_INFO *
CreateSockInfo(
    PROVIDER  *Provider, 
    SOCKET     ProviderSocket, 
    SOCK_INFO *Inherit, 
    BOOL       Insert,
    int       *lpErrno
    )
{
    SOCK_INFO   *NewInfo = NULL;

    NewInfo = (SOCK_INFO *) LspAlloc(
            sizeof( SOCK_INFO ),
            lpErrno
            );
    if ( NULL == NewInfo )
    {
        dbgprint("HeapAlloc() failed: %d", GetLastError());
       *lpErrno = WSAENOBUFS;
        goto cleanup;
    }

    //
    // Initialize the fields to default values
    //
    NewInfo->ProviderSocket     = ProviderSocket;
    NewInfo->bClosing           = FALSE;
    NewInfo->dwOutstandingAsync = 0;
    NewInfo->BytesRecv          = 0;
    NewInfo->BytesSent          = 0;
    NewInfo->Provider           = Provider;
    NewInfo->hWnd               = (Inherit ? Inherit->hWnd : 0);
    NewInfo->uMsg               = (Inherit ? Inherit->uMsg : 0);

    __try
    {
        InitializeCriticalSection( &NewInfo->SockCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        *lpErrno = WSAENOBUFS;
        goto cleanup;
    }

    if ( TRUE == Insert )
        InsertSocketInfo(Provider, NewInfo);

    return NewInfo;

cleanup:

    if ( NULL != NewInfo )
        LspFree( NewInfo );

    return NULL;
}

//
// Function: FreeSockInfo
//
// Description:
//    This routine frees the socket context structure.
//
void 
FreeSockInfo(
    SOCK_INFO *info
    )
{
    DeleteCriticalSection( &info->SockCritSec );
    LspFree( info );

    return;
}

//
// Function: InsertSocketInfo
//
// Description:
//    We keep track of all the sockets created for a particulare provider.
//    This routine inserts a newly created socket (and its SOCK_INFO) into
//    the list.
//
void 
InsertSocketInfo(
    PROVIDER  *provider, 
    SOCK_INFO *sock
    )
{
    if ( ( NULL == provider ) || ( NULL == sock ) )
    {
        dbgprint("InsertSocketInfo: PROVIDER or SOCK_INFO == NULL!");
        goto cleanup;
    }

    EnterCriticalSection( &provider->ProviderCritSec );

    InsertTailList( &provider->SocketList, &sock->Link );

    LeaveCriticalSection( &provider->ProviderCritSec );

    SetEvent( gAddContextEvent );

cleanup:

    return;
}

// 
// Function: RemoveSocketInfo
//
// Description:
//    This function removes a given SOCK_INFO structure from the referenced
//    provider. It doesn't free the structure, it just removes it from the 
//    list.
//
void 
RemoveSocketInfo(
    PROVIDER  *provider, 
    SOCK_INFO *si
    )
{
    EnterCriticalSection( &provider->ProviderCritSec );

    RemoveEntryList( &si->Link );

    LeaveCriticalSection( &provider->ProviderCritSec );

    return;
}

//
// Function: CloseAndFreeSocketInfo
//
// Description:
//    Closes all sockets belonging to the specified provider and frees
//    the context information. If the lower provider socket is still 
//    valid, set an abortive linger, and close the socket.
//
void 
CloseAndFreeSocketInfo(
    PROVIDER *provider,
    BOOL      processDetach
    )
{
    LIST_ENTRY   *entry = NULL;
    SOCK_INFO    *si = NULL;
    struct linger linger;
    int           Error, 
                  ret;

    ASSERT( provider );

    linger.l_onoff  = 1;
    linger.l_linger = 0;

    // Walk the list of sockets
    while ( !IsListEmpty( &provider->SocketList ) )
    {
        entry = RemoveHeadList( &provider->SocketList );

        ASSERT( entry );

        si = CONTAINING_RECORD( entry, SOCK_INFO, Link );

        if ( ( !processDetach ) || 
             ( provider->NextProvider.ProtocolChain.ChainLen == BASE_PROTOCOL ) )
        {

            ASSERT( provider->NextProcTable.lpWSPSetSockOpt );

            // Set the abortive linger
            ret = provider->NextProcTable.lpWSPSetSockOpt(
                    si->ProviderSocket,
                    SOL_SOCKET,
                    SO_LINGER,
                    (char *) &linger,
                    sizeof(linger),
                    &Error
                    );
            if ( SOCKET_ERROR != ret )
            {
                ASSERT( provider->NextProcTable.lpWSPCloseSocket );

                // Close the lower provider socket
                ret = provider->NextProcTable.lpWSPCloseSocket(
                        si->ProviderSocket,
                        &Error
                        );
                if ( SOCKET_ERROR == ret )
                {
                    dbgprint("WSPCloseSocket() on handle %d failed: %d", si->ProviderSocket, Error);
                }
#ifdef DEBUG
                else
                {
                    dbgprint("Successfully closed socket %d", si->ProviderSocket);
                }
#endif
            }
#ifdef DEBUG
            else
            {
                dbgprint("WSPSetSockOpt(SO_LINGER) failed: %d", Error);
            }
#endif
        }

        ASSERT( gMainUpCallTable.lpWPUCloseSocketHandle );

        // Close the layered handle
        gMainUpCallTable.lpWPUCloseSocketHandle(
                si->LayeredSocket, 
               &Error
                );

        // Free the context structure
        FreeSockInfo( si );
    }

    return;
}

//
// Function: FindSockInfoFromProviderSocket
//
// Description:
//      This routine searches the list of socket context structures in a given
//      provider that matches the passed in provider (lower layer) socket. If found
//      the SOCK_INFO structure is returned; otherwise, NULL is returned.
//
SOCK_INFO *
FindSockInfoFromProviderSocket(
    PROVIDER   *provider,
    SOCKET      socket
    )
{
    LIST_ENTRY *lptr = NULL;
    SOCK_INFO  *si = NULL;

    ASSERT( provider );

    if ( IsListEmpty( &provider->SocketList ) )
    {
        dbgprint( "FindSockInfoFromProviderSocket: Empty SOCK_INFO list!" );
        goto cleanup;
    }

    for(lptr = provider->SocketList.Flink ; lptr != &provider->SocketList ; lptr = lptr->Flink )
    {
        si = CONTAINING_RECORD( lptr, SOCK_INFO, Link );

        if ( socket == si->ProviderSocket )
            break;

        si = NULL;
    }

cleanup:

    return si;
}

//
// Function: GetCallerSocket
//
// Description:
//    This function returns the SOCK_INFO structure for the given
//    provider socket. If provider is NULL then we'll search all
//    providers for the socket info. This routine is only used
//    in handling asynchronous window messages (WSAAsyncSelect)
//    since the window handler receives only the provider's socket
//    and we need to find the associated context structure.
//
SOCK_INFO *
GetCallerSocket(
    PROVIDER *provider, 
    SOCKET    ProviderSock
    )
{
    SOCK_INFO *si = NULL;

    EnterCriticalSection( &gCriticalSection );

    if ( NULL != provider )
    {
        // If we know the provider just search its list of sockets
        si = FindSockInfoFromProviderSocket( provider, ProviderSock );
    }
    else
    {
        // Don't know the provider so we must search all of them
        for(INT i=0; i < gLayerCount ;i++)
        {
            si = FindSockInfoFromProviderSocket( &gBaseInfo[ i ], ProviderSock );
            if ( NULL != si )
                break;
        }
    }

    LeaveCriticalSection( &gCriticalSection );

    return si;
}
