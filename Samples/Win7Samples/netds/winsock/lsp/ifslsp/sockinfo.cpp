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
//    This sample illustrates an IFS based LSP which implements basic proxying
//    capabilities.
//
//    This file contains routines associated with the SOCKET_CONTEXT structure. This
//    structure maintains the mapping between the upper layer's socket and the
//    corresponding lower layer's socket. The routines in this file are for allocating, 
//    linked list management, etc.
//    

#include "lspdef.h"

//
// Function: FindAndRefSocketContext
//
// Description:
//    This routine grabs the LSP critical seciton to lookup the socket context
//    and increase its ref count. Any operation on the socket context holds
//    the critical section so that it cannot be freed while its state changes.
//
SOCKET_CONTEXT *
FindSocketContext(
    SOCKET  s,
    BOOL    Remove
    )
{
    SOCKET_CONTEXT  *SocketContext = NULL,
               *info = NULL;
    LIST_ENTRY *lptr = NULL;
    int         i;

    EnterCriticalSection( &gCriticalSection );

    for(i=0; i < gLayerCount ;i++)
    {
        EnterCriticalSection( &gLayerInfo[ i ].ProviderCritSec );

        for(lptr = gLayerInfo[ i ].SocketList.Flink ;
            lptr != &gLayerInfo[ i ].SocketList ;
            lptr = lptr->Flink )
        {
            info = CONTAINING_RECORD( lptr, SOCKET_CONTEXT, Link );

            if ( s == info->Socket )
            {
                SocketContext = info;
                
                if ( TRUE == Remove )
                {
                    RemoveEntryList( &info->Link );
                }
                break;
            }
        }

        LeaveCriticalSection( &gLayerInfo[ i ].ProviderCritSec );

        if ( NULL != SocketContext )
            break;
    }

    LeaveCriticalSection( &gCriticalSection );

    return SocketContext;
}

//
// Function: CreateSockInfo
//
// Description:
//    Allocates a new socket info context structure and initializes the fields
//    except for the LayeredSocket field. The context must be allocated first,
//    then the layered socket is created (with the SOCKET_CONTEXT structure as the
//    context information), and then the LayeredSocket field is set. If
//    the Inherit context is provided, information is copied to the new socket
//    context structure (such as with WSPAccept). If the Insert flag is TRUE
//    then the context is automatically inserted into the list of sockets
//    for the given provider. If not then the caller must insert the context
//    (WSPAccept does this to ensure all fields of the context are valid
//    including LayeredSocket before insertion so that the async thread
//    handler will work properly).
//
SOCKET_CONTEXT *
CreateSocketContext(
    PROVIDER  *Provider, 
    SOCKET     Socket, 
    int       *lpErrno
    )
{
    SOCKET_CONTEXT   *newContext = NULL;

    newContext = (SOCKET_CONTEXT *) LspAlloc(
            sizeof( SOCKET_CONTEXT ),
            lpErrno
            );
    if ( NULL == newContext )
    {
        dbgprint("CreateSocketContext: LspAlloc failed: %d", *lpErrno );
        goto cleanup;
    }

    newContext->Socket     = Socket;
    newContext->Provider   = Provider;
    newContext->Proxied    = FALSE;

    EnterCriticalSection( &Provider->ProviderCritSec );

    InsertHeadList( &Provider->SocketList, &newContext->Link );

    LeaveCriticalSection( &Provider->ProviderCritSec );

    return newContext;

cleanup:

    return NULL;
}

//
// Function: FreeSockInfo
//
// Description:
//    This routine frees the socket context structure.
//
void 
FreeSocketContext(
    PROVIDER       *Provider,
    SOCKET_CONTEXT *Context
    )
{
    EnterCriticalSection( &Provider->ProviderCritSec );

    RemoveEntryList( &Context->Link );
    LspFree( Context );

    LeaveCriticalSection( &Provider->ProviderCritSec );

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
FreeSocketContextList(
        PROVIDER *provider
        )
{
    LIST_ENTRY     *lptr = NULL;
    SOCKET_CONTEXT *context = NULL;

    ASSERT( provider );

    // Walk the list of sockets
    while ( !IsListEmpty( &provider->SocketList ) )
    {
        lptr = RemoveHeadList( &provider->SocketList );

        ASSERT( lptr );

        context = CONTAINING_RECORD( lptr, SOCKET_CONTEXT, Link );

        // Context is already removed so just free it
        LspFree( context );
    }

    return;
}
