// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: spi.cpp
//
// Description:
//
//  This sample illustrates how to develop an IFS Layered Service Provider (LSP) which
//  functions as a TCP proxy client. Because this LSP only needs to intercept a handful
//  of functions like WSPSocket and WSPConnect as well as the fact it does not need to
//  intercept operation completion (e.g. intercept a WSPRecv after it completes but 
//  before the application receives the indication), the LSP can be implemented as an
//  IFS provider. This means that the socket handle returned from this LSP is simply
//  the provider's handle below this LSP. However, this does mean this LSP must be
//  installed over other IFS providers only -- it will not function if it is installed
//  over a non-IFS provider.
//
//  This file contains the Winsock Service Provider Interface (SPI) functions that
//  the LSP is overriding. The LSP only overrides a few functions (those related to
//  performing as a TCP proxy client) and the remaining functions are the lower
//  provider's. That is, when this LSP is loaded, it loads the lower provider's
//  function table, copies it as its own, and overrides only a few functions. This
//  proc table is then returned to the caller.
//    
#include "lspdef.h"
#include <strsafe.h>

#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4127)       // Disable "conditional expression is constant" warning

#define DEFAULT_PRINT_BUFFER    512

////////////////////////////////////////////////////////////////////////////////
//
// Globals used across files
//
////////////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION    gCriticalSection;   // Critical section to protect startup/cleanup
WSPUPCALLTABLE      gMainUpCallTable;   // Winsock upcall table
LPPROVIDER          gLayerInfo = NULL;  // Provider information for each layer under us
int                 gLayerCount = 0;    // Number of providers layered over

////////////////////////////////////////////////////////////////////////////////
//
// Globals local to this file
//
////////////////////////////////////////////////////////////////////////////////

static BOOL gDetached = FALSE;      // Indicates if process is detaching from DLL
static int  gStartupCount = 0;      // Global startup count (for every WSPStartup call)

// Parses a buffer looking for an HTTP GET request and returns the requested URL
int
FindUrl( 
        __in_ecount(buflen) char *buf,
        int buflen,
        __out char **start
        );

////////////////////////////////////////////////////////////////////////////////
//
// SPI Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: DllMain
//
// Description:
//    Provides initialization when the LSP DLL is loaded. In our case we simply,
//    initialize some critical sections used throughout the DLL.
//
BOOL WINAPI 
DllMain(
        IN HINSTANCE hinstDll, 
        IN DWORD dwReason, 
        LPVOID lpvReserved
       )
{
    UNREFERENCED_PARAMETER( hinstDll );
    UNREFERENCED_PARAMETER( lpvReserved );

    switch (dwReason)
    {

        case DLL_PROCESS_ATTACH:
            //
            // Initialize some critical section objects 
            //
            __try
            {
                InitializeCriticalSection( &gCriticalSection );
                InitializeCriticalSection( &gDebugCritSec );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                goto cleanup;
            }
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            gDetached = TRUE;

            EnterCriticalSection( &gCriticalSection );
            if ( NULL != gLayerInfo )
            {
                int Error;

                // Free LSP structures if still present as well as call WSPCleanup
                //    for all providers this LSP loaded
                FreeLspProviders( gLayerInfo, gLayerCount, &Error );
                gLayerInfo = NULL;
                gLayerCount = 0;
            }
            LeaveCriticalSection( &gCriticalSection );

            DeleteCriticalSection( &gCriticalSection );
            DeleteCriticalSection( &gDebugCritSec );

            break;
    }

    return TRUE;

cleanup:

    return FALSE;
}

// 
// Function: WSPCleanup
//
// Description:
//    Decrement the entry count. If equal to zero then we can prepare to have us
//    unloaded so all resources should be freed
//
int WSPAPI 
WSPCleanup(
        LPINT lpErrno  
        )
{
    int        rc = SOCKET_ERROR;

    if ( gDetached )
    {
        rc = NO_ERROR;
        goto cleanup;
    }

    //
    // Grab the DLL global critical section
    //
    EnterCriticalSection( &gCriticalSection );

    // Verify WSPStartup has been called
    if ( 0 == gStartupCount )
    {
        *lpErrno = WSANOTINITIALISED;
        goto cleanup;
    }

    // Decrement the global entry count
    gStartupCount--;

    if ( 0 == gStartupCount )
    {
        // Free LSP structures if still present as well as call WSPCleanup
        //    for all providers this LSP loaded
        FreeLspProviders( gLayerInfo, gLayerCount, lpErrno );
        gLayerInfo = NULL;
        gLayerCount = 0;
    }
    
    rc = NO_ERROR;

cleanup:

    LeaveCriticalSection( &gCriticalSection );

    return rc;
}

//
// Function: WSPCloseSocket
//
// Description:
//    The LSP captures the WSPCloseSocket call to know when it can free the 
//    socket context structure created for every socket.
//
int WSPAPI
WSPCloseSocket(
        SOCKET s,
        LPINT  lpErrno
        )
{
    SOCKET_CONTEXT *sockContext = NULL;
    int             rc = SOCKET_ERROR;

    // Find the socket context and remove it from the provider's list of sockets
    sockContext = FindSocketContext( s, TRUE );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPCloseSocket );

    // Pass the socket down to close it
    rc = sockContext->Provider->NextProcTable.lpWSPCloseSocket(
            s,
            lpErrno
            );

    // Just free the structure as its alreayd removed from the provider's list
    LspFree( sockContext );

cleanup:

    return rc;
}

//
// Function: WSPConnect
//
// Description:
//    This routine establishes a connection on a socket. For the LSP it first
//    determines if the destination is to be proxied to a different address.
//    Once the "correct" destination is determined, the connect call is passed
//    to the lower provider.
//
int WSPAPI 
WSPConnect(
        SOCKET                s,
        const struct sockaddr FAR * name,
        int                   namelen,
        LPWSABUF              lpCallerData,
        LPWSABUF              lpCalleeData,
        LPQOS                 lpSQOS,
        LPQOS                 lpGQOS,
        LPINT                 lpErrno
        )
{
    SOCKET_CONTEXT *sockContext = NULL;
    SOCKADDR       *proxyAddr = NULL;
    int             proxyLen = 0,
                    rc = SOCKET_ERROR;

    // Find the socket context
    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPConnect );

    FindDestinationAddress( sockContext, name, namelen, &proxyAddr, &proxyLen );

    rc = sockContext->Provider->NextProcTable.lpWSPConnect(
            s,
            proxyAddr, 
            proxyLen, 
            lpCallerData, 
            lpCalleeData,
            lpSQOS, 
            lpGQOS, 
            lpErrno
            );

cleanup:

    return rc;
}

//
// Function: WSPGetPeerName
//
// Description:
//    Since the LSP is proxying the remote address, this function needs to be 
//    intercepted in order to return the address that the application thinks its
//    connected to.
//
int WSPAPI 
WSPGetPeerName(  
        SOCKET          s,
        struct sockaddr FAR * name,
        LPINT           namelen,
        LPINT           lpErrno
        )
{
    SOCKET_CONTEXT *sockContext = NULL;
    int             rc = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        goto cleanup;
    }

    // If the connection has been proxied, return the address the application
    //    originally tried to connect to.
    if ( TRUE == sockContext->Proxied )
    {
        __try
        {
            // Verify buffer is large enough for underlying address structure
            if ( *namelen < sockContext->AddressLength )
            {
                *namelen = sockContext->AddressLength;
                *lpErrno = WSAEFAULT;
                goto cleanup;
            }

            memcpy( name, &sockContext->OriginalAddress, *namelen );
            *namelen = sockContext->AddressLength;
        } 
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        return NO_ERROR;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPGetPeerName );

    rc = sockContext->Provider->NextProcTable.lpWSPGetPeerName(
            s,
            name,
            namelen, 
            lpErrno
            );

cleanup:

    return rc;
}

//
// Function: WSPGetSockOpt
//
// Description:
//
//
int WSPAPI
WSPGetSockOpt(
        SOCKET s,
        int level,
        int optname,
        __out_bcount(*optlen) char FAR * optval,
        __inout LPINT optlen,
        LPINT lpErrno
        )
{
    SOCKET_CONTEXT *sockContext = NULL;
    int             rc = NO_ERROR;

    // Retrieve the socket context
    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        rc = SOCKET_ERROR;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPGetSockOpt );

    if ( ( SOL_SOCKET == level ) && (
            ( SO_PROTOCOL_INFOA == optname ) ||
            ( SO_PROTOCOL_INFOW == optname )
         )
       )
    {
        __try
        {
            switch ( optname )
            {
                case SO_PROTOCOL_INFOA:
                    if (  *optlen < sizeof( WSAPROTOCOL_INFOA ) )
                    {
                        *optlen = sizeof( WSAPROTOCOL_INFOA );
                        *lpErrno = WSAEFAULT;
                        rc = SOCKET_ERROR;
                        goto cleanup;
                    }

                    *optlen = sizeof( WSAPROTOCOL_INFOA );
                    memcpy( optval, &sockContext->Provider->LayerProvider,
                            sizeof( WSAPROTOCOL_INFOA ) );

                    break;

                case SO_PROTOCOL_INFOW:
                    if ( *optlen < sizeof( WSAPROTOCOL_INFOW ) )
                    {
                        *optlen = sizeof( WSAPROTOCOL_INFOW );
                        *lpErrno = WSAEFAULT;
                        rc = SOCKET_ERROR;
                        goto cleanup;
                    }

                    *optlen = sizeof( WSAPROTOCOL_INFOW );
                    memcpy( optval, &sockContext->Provider->LayerProvider,
                            sizeof( WSAPROTOCOL_INFOW ) );

                    break;

            }
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            *lpErrno = WSAEFAULT;
            rc = SOCKET_ERROR;
            goto cleanup;
        }

        if ( SO_PROTOCOL_INFOA == optname )
        {
            WideCharToMultiByte( 
                    CP_ACP, 
                    0,
                    sockContext->Provider->LayerProvider.szProtocol,
                    -1,
                    ( (WSAPROTOCOL_INFOA *)optval )->szProtocol,
                    WSAPROTOCOL_LEN+1,
                    NULL,
                    NULL
                    );
        }
    }
    else
    {
        rc = sockContext->Provider->NextProcTable.lpWSPGetSockOpt(
                s,
                level,
                optname,
                optval,
                optlen,
                lpErrno
                );
    }

cleanup:

    return rc;
}

//
// Function: WSPIoctl
//
// Description:
//    Since the LSP proxies connection requests, the LSP needs to intercept the
//    ConnectEx function. This routine returns the LSPs ConnectEx function when
//    the application requests it.
//
int WSPAPI 
WSPIoctl(
        SOCKET          s,
        DWORD           dwIoControlCode,
        LPVOID          lpvInBuffer,
        DWORD           cbInBuffer,
        LPVOID          lpvOutBuffer,
        DWORD           cbOutBuffer,
        LPDWORD         lpcbBytesReturned,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        LPWSATHREADID   lpThreadId,
        LPINT           lpErrno
        )
{
    SOCKET_CONTEXT *sockContext = NULL;
    GUID            ConnectExGuid = WSAID_CONNECTEX;
    int             rc = NO_ERROR;

    // If loading an extension function, check for ConnectEx
    if ( SIO_GET_EXTENSION_FUNCTION_POINTER == dwIoControlCode )
    {
        if ( 0 == memcmp( lpvInBuffer, &ConnectExGuid, sizeof( GUID ) ) )
        {
            // Return a pointer to our intermediate extension function
            __try 
            {
                if ( cbOutBuffer < sizeof( LPFN_CONNECTEX ) )
                {
                    *lpcbBytesReturned = sizeof( LPFN_CONNECTEX );
                    *lpErrno = WSAEFAULT;
                    rc = SOCKET_ERROR;
                    goto cleanup;
                }
                *lpcbBytesReturned = sizeof( LPFN_CONNECTEX );
                *((DWORD_PTR *)lpvOutBuffer) = (DWORD_PTR) ExtConnectEx;
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                *lpErrno = WSAEFAULT;
                rc = SOCKET_ERROR;
            }
            return rc;
        }
    }

    // Retrieve the socket context
    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        rc = SOCKET_ERROR;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPIoctl );

    // Pass the call to the lower layer
    rc = sockContext->Provider->NextProcTable.lpWSPIoctl(
            s,
            dwIoControlCode, 
            lpvInBuffer,
            cbInBuffer, 
            lpvOutBuffer, 
            cbOutBuffer, 
            lpcbBytesReturned, 
            lpOverlapped, 
            lpCompletionRoutine, 
            lpThreadId, 
            lpErrno
            );

cleanup:

    return rc;
}

// 
// Function: WSPSend
//
// Description:
//      This function implements the WSPSend function for the IFS LSP. This routine
//      simply parses the send buffer for an HTTP GET request. If one is found it
//      is simply displayed to the debugger. This illustrates how to parse data
//      buffers. Note that this samply only intercepts the WSPSend routine since
//      we're interested in only HTTP TCP traffic. If we wanted to parse datagram
//      oriented protocols we should then also intercept WSPSendTo, WSASendMsg,
//      and TransmitPackets. Note that we don't intercept TransmitFile or 
//      TransmitPackets for this HTTP parsing since the client HTTP sides do not
//      use those APIs for sending requests (i.e. these extension functions are
//      typically used in the server response).
//
int WSPAPI
WSPSend(
        SOCKET          s,
        LPWSABUF        lpBuffers,
        DWORD           dwBufferCount,
        LPDWORD         lpNumberOfBytesSent,
        DWORD           dwFlags,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        LPWSATHREADID   lpThreadId,
        LPINT           lpErrno
       )
{
    SOCKET_CONTEXT *sockContext = NULL;
    DWORD           i;
    char           *start = NULL,
                    urlstr[ DEFAULT_PRINT_BUFFER ];
    int             rc = SOCKET_ERROR,
                    len;

    //
    // Find our provider socket corresponding to this one
    //
    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        goto cleanup;
    }

    // Parse the send buffer and look for HTTP GET requests
    __try
    {
        for(i=0; i < dwBufferCount ;i++)
        {
            len = FindUrl( lpBuffers[ i ].buf, lpBuffers[ i ].len, &start );
            if ( ( len > 0 ) && ( len+1 < DEFAULT_PRINT_BUFFER ) )
            {
                if ( FAILED (StringCchCopyN( urlstr, DEFAULT_PRINT_BUFFER, start, len+1 ) ) )
                {
                    *lpErrno = WSAEFAULT;
                    goto cleanup;
                }

                if ( len >= DEFAULT_PRINT_BUFFER )
                    urlstr[ DEFAULT_PRINT_BUFFER-1 ] = '\0';
                else
                    urlstr[len] = '\0';

                // URL can be logged but this just displays it to the debugger
                dbgprint("Found URL: '%s'", urlstr );
            }
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        dbgprint("WSPSend: ***access violation ***");
        *lpErrno = WSAEFAULT;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPSend );

    // Just pass the request along to the lower provider. NOTE: If we choose to
    //    modify the data things get a bit trickier if we substitute our own send
    //    buffer since we would need to be in the data notification path in order
    //    to know when we are able to free that memory (i.e. the lower layer has
    //    processed it and is done). In this case a non-IFS LSP is more appropriate
    //    since it intercepts all IO completion notifications.

    rc = sockContext->Provider->NextProcTable.lpWSPSend(
            s,
            lpBuffers,
            dwBufferCount, 
            lpNumberOfBytesSent,
            dwFlags,
            lpOverlapped,
            lpCompletionRoutine,
            lpThreadId,
            lpErrno
            );
cleanup:

    return rc;
}

//
// Function: WSPSocket
//
// Description:
//    This routine creates a socket. For an IFS LSP the lower provider's socket
//    handle is returned to the uppler layer. When a socket is created, a socket
//    context structure is created for the socket returned from the lower provider.
//    This context is used if the socket is later connected to a proxied address.
//
SOCKET WSPAPI 
WSPSocket(
        int                 af,
        int                 type,
        int                 protocol,
        LPWSAPROTOCOL_INFOW lpProtocolInfo,
        GROUP               g,
        DWORD               dwFlags,
        LPINT               lpErrno
        )
{
    WSAPROTOCOL_INFOW   InfoCopy = {0};
    SOCKET_CONTEXT     *sockContext = NULL;
    PROVIDER           *lowerProvider = NULL;
    SOCKET              nextProviderSocket = INVALID_SOCKET,
                        sret = INVALID_SOCKET;
    int                 rc;

    // Find the LSP entry which matches the given provider info
    lowerProvider = FindMatchingLspEntryForProtocolInfo(
            lpProtocolInfo,
            gLayerInfo,
            gLayerCount
            );
    if ( NULL == lowerProvider )
    {
        dbgprint("WSPSocket: FindMatchingLspEntryForProtocolInfo failed!" );
        goto cleanup;
    }

    if ( 0 == lowerProvider->StartupCount ) 
    {
        rc = InitializeProvider( lowerProvider, MAKEWORD(2,2), lpProtocolInfo, 
               gMainUpCallTable, lpErrno );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint("WSPSocket: InitializeProvider failed: %d", *lpErrno);
            goto cleanup;
        }
    }

    // If the next layer is a base, substitute the provider structure with the
    //    base provider's
    if ( BASE_PROTOCOL == lowerProvider->NextProvider.ProtocolChain.ChainLen )
    {
        memcpy( &InfoCopy, &lowerProvider->NextProvider, sizeof( InfoCopy ) );
        InfoCopy.dwProviderReserved = lpProtocolInfo->dwProviderReserved;
        if (af == FROM_PROTOCOL_INFO)
            InfoCopy.iAddressFamily = lpProtocolInfo->iAddressFamily;
        if (type == FROM_PROTOCOL_INFO)
            InfoCopy.iSocketType = lpProtocolInfo->iSocketType;
        if (protocol == FROM_PROTOCOL_INFO)
            InfoCopy.iProtocol = lpProtocolInfo->iProtocol;
        
        lpProtocolInfo = &InfoCopy;
    }

    ASSERT( lowerProvider->NextProcTable.lpWSPSocket );

    //
    // Create the socket from the lower layer
    //
    nextProviderSocket = lowerProvider->NextProcTable.lpWSPSocket(
            af, 
            type, 
            protocol, 
            lpProtocolInfo,
            g, 
            dwFlags,
            lpErrno
            );
    if ( INVALID_SOCKET == nextProviderSocket )
    {
        dbgprint("WSPSocket: NextProcTable.WSPSocket failed: %d", *lpErrno);
        goto cleanup;
    }

    //
    // Create the context information to be associated with this socket
    //
    sockContext = CreateSocketContext(
            lowerProvider,
            nextProviderSocket,
            lpErrno
            );
    if ( NULL == sockContext )
    {
        dbgprint( "WSPSocket: CreateSocketContext failed: %d", *lpErrno );
        goto cleanup;
    }

    //
    // Associate ownership of this handle with our LSP
    //
    sret = gMainUpCallTable.lpWPUModifyIFSHandle(
            lowerProvider->LayerProvider.dwCatalogEntryId,
            nextProviderSocket,
            lpErrno
            );
    if ( INVALID_SOCKET == sret )
    {
        dbgprint( "WSPSocket: WPUModifyIFSHandle failed: %d", *lpErrno );
        goto cleanup;
    }

    ASSERT( sret == nextProviderSocket );

    return nextProviderSocket;

cleanup:

    // If an error occured close the socket if it was already created
    if ( ( NULL != sockContext ) && ( NULL != lowerProvider ) &&
         ( INVALID_SOCKET != nextProviderSocket ) )
    {
        rc = lowerProvider->NextProcTable.lpWSPCloseSocket(
                nextProviderSocket,
                lpErrno
                );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint( "WSPSocket: WSPCloseSocket failed: %d", *lpErrno );
        }

    }
    if ( ( NULL != sockContext ) && ( NULL != lowerProvider ) )
        FreeSocketContext( lowerProvider, sockContext );

    return INVALID_SOCKET;
}

//
// Function: WSPAccept
//
// Description:
//    This routine must be intercepted for connection oriented providers. This is
//    necessary since if an accepted socket is used by one of the other functions
//    the IFS LSP is intercepting then state *must* be maintained; otherwise, the
//    search for the context would fail and an error be returned.
//
SOCKET WSPAPI 
WSPAccept(
        SOCKET               s,
        struct sockaddr FAR *addr,
        LPINT                addrlen,
        LPCONDITIONPROC      lpfnCondition,
        DWORD_PTR            dwCallbackData,
        LPINT                lpErrno
        )
{
    SOCKET_CONTEXT     *sockContext = NULL,
                       *acceptContext = NULL;
    SOCKET              acceptProviderSocket = INVALID_SOCKET,
                        sret;
    int                 rc;

    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        *lpErrno = WSAENOTSOCK;
        goto cleanup;
    }

    ASSERT( sockContext->Provider->NextProcTable.lpWSPAccept );

    //
    // Create the socket from the lower layer
    //
    acceptProviderSocket = sockContext->Provider->NextProcTable.lpWSPAccept(
            s, 
            addr, 
            addrlen, 
            lpfnCondition,
            dwCallbackData,
            lpErrno
            );
    if ( INVALID_SOCKET == acceptProviderSocket )
    {
        dbgprint("WSPAccept: NextProcTable.WSPAccept failed: %d", *lpErrno);
        goto cleanup;
    }

    //
    // Create the context information to be associated with this socket
    //
    acceptContext = CreateSocketContext(
            sockContext->Provider,
            acceptProviderSocket,
            lpErrno
            );
    if ( NULL == sockContext )
    {
        dbgprint( "WSPAccept: CreateSocketContext failed: %d", *lpErrno );
        goto cleanup;
    }

    //
    // Associate ownership of this handle with our LSP
    //
    sret = gMainUpCallTable.lpWPUModifyIFSHandle(
            sockContext->Provider->LayerProvider.dwCatalogEntryId,
            acceptProviderSocket,
            lpErrno
            );
    if ( INVALID_SOCKET == sret )
    {
        dbgprint( "WSPAccept: WPUModifyIFSHandle failed: %d", *lpErrno );
        goto cleanup;
    }

    ASSERT( sret == acceptProviderSocket );

    return acceptProviderSocket;

cleanup:

    // If an error occured close the socket if it was already created
    if ( ( NULL != sockContext ) && ( NULL != acceptContext ) &&
         ( INVALID_SOCKET != acceptProviderSocket ) )
    {
        rc = sockContext->Provider->NextProcTable.lpWSPCloseSocket(
                acceptProviderSocket,
                lpErrno
                );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint( "WSPAccept: WSPCloseSocket failed: %d", *lpErrno );
        }

    }
    if ( ( NULL != sockContext ) && ( NULL != acceptContext ) )
        FreeSocketContext( sockContext->Provider, acceptContext );

    return INVALID_SOCKET;
}

//
// Function: WSPStartup
//
// Description:
//    This function initializes the LSP. If this is the first time WSPStartup
//    is called, the LSP builds an array of its providers. The PROVIDER structure
//    contains an LSP protocol chain and the Winsock provider it is layered over.
//    Once this structure is created, the LSP determines which LSP protocol chain
//    matches the lpProtocolInfo passed into it. Note that the IFS LSP *requires*
//    that WSPStartup is invoked for *each* of its layered protocol chains. This
//    is required since the IFS LSP intercepts a subset of all the SPI functions.
//    For those functions the LSP isn't interested in intercepting, the LSP passes
//    the lower provider's functions pointers. Consider the following example:
//
//      1.  Install this IFS LSP (LSP2 in the diagram) over a BASE TCP/IPv4
//          provider as well as another IFS LSP UDP/IPv4 (LSP1 in the diagram).
//          ______________________ _______________________
//          |  IFS LSP2 TCP/IPv4 | |  IFS LSP2 UDP/IPv4  |
//          |____________________| |_____________________|
//          |  BASE TCP/IPv4     | |  IFS LSP1 UDP/IPv4  |
//          |____________________| |_____________________|
//
//      2. If LSP2 WSPStartup is invoked with TCP/IPv4 then LSP2 loads the
//         BASE TCP/IPv4 provider and returns some of its function pointers to
//         the caller (i.e. the functions that LSP2 doesn't intercept)
//
//      3. Now, what if the process creates a UDP/IPv4 socket? If both of LSP2's
//         layered protocol entries were installed in a single WSCInstallProvider
//         call (under a single GUID), LSP2's WSPStartup is *NOT* invoked and this
//         UDP socket will have a proc table that points into the BASE TCP/IPv4
//         provider.
//
//         However, if the two LSP2 entries were installed in two calls to 
//         WSCInstallProvider with two GUIDs, then when a UDP/IPv4 socket is created
//         LSP2's WSPStartup is invoked again at which point the correct proc
//         table is built using the LSP1's function pointers which LSP2 doesn't wish
//         to override.
//
int WSPAPI 
WSPStartup(
    WORD                wVersion,
    LPWSPDATA           lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE      UpCallTable,
    LPWSPPROC_TABLE     lpProcTable
    )
{
    PROVIDER           *loadProvider = NULL;
    int                 Error = WSAEPROVIDERFAILEDINIT,
                        rc;

    EnterCriticalSection( &gCriticalSection );

    // The first time the startup is called, create our heap and allocate some
    //    data structures for tracking the LSP providers
    if ( 0 == gStartupCount )
    {
        // Create the heap for all LSP allocations
        rc = LspCreateHeap( &Error );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint("WSPStartup: LspCreateHeap failed: %d", Error );
            goto cleanup;
        }

        // Find this LSP's entries in the Winsock catalog and build a map of them
        rc = FindLspEntries( &gLayerInfo, &gLayerCount, &Error );
        if ( FALSE == rc )
        {
            dbgprint("WSPStartup: FindLspEntries failed: %d", Error );
            goto cleanup;
        }

        // Save off upcall table - this should be the same across all WSPStartup calls
        memcpy( &gMainUpCallTable, &UpCallTable, sizeof( gMainUpCallTable ) );

    }

    // Find the matching LSP provider for the requested protocol info passed in.
    //    This can either be an LSP layered over use or an entry belonging to this
    //    LSP. Note that the LSP startup gets called for each LSP layered protocol
    //    entry with a unique GUID. Because of this each layered protocol entry for
    //    the IFS LSP should be installed with its own unique GUID.
    loadProvider = FindMatchingLspEntryForProtocolInfo(
            lpProtocolInfo,
            gLayerInfo,
            gLayerCount,
            TRUE
            );
    if ( NULL == loadProvider )
    {
        dbgprint("WSPStartup: FindMatchingLspEntryForProtocolInfo failed!");
        ASSERT( 0 );
        goto cleanup;
    }

    // If this is the first time to "load" this particular provider, initialize
    //    the lower layer, etc.
    if ( 0 == loadProvider->StartupCount )
    {

        rc = InitializeProvider( loadProvider, wVersion, lpProtocolInfo, 
                UpCallTable, &Error );
        if ( SOCKET_ERROR == rc )
        {
            dbgprint("WSPStartup: InitializeProvider failed: %d", Error );
            goto cleanup;
        }

    }

    gStartupCount++;

    // Build the proc table to return to the caller
    memcpy( lpProcTable, &loadProvider->NextProcTable, sizeof( *lpProcTable ) );

    // Override only those functions the LSP wants to intercept
    lpProcTable->lpWSPAccept        = WSPAccept;
    lpProcTable->lpWSPCleanup       = WSPCleanup;
    lpProcTable->lpWSPCloseSocket   = WSPCloseSocket;
    lpProcTable->lpWSPConnect       = WSPConnect;
    lpProcTable->lpWSPGetPeerName   = WSPGetPeerName;
    lpProcTable->lpWSPGetSockOpt    = WSPGetSockOpt;
    lpProcTable->lpWSPIoctl         = WSPIoctl;
    lpProcTable->lpWSPSend          = WSPSend;
    lpProcTable->lpWSPSocket        = WSPSocket;

    memcpy( lpWSPData, &loadProvider->WinsockVersion, sizeof( *lpWSPData ) );

    Error = NO_ERROR;

cleanup:

    LeaveCriticalSection( &gCriticalSection );

    return Error;
}

////////////////////////////////////////////////////////////////////////////////
//
// Helper Function Implementation
//
////////////////////////////////////////////////////////////////////////////////


//
// Function: FindDestinationAddress
//
// Description:
//      This function is invoked whenver a connection request is made by the upper
//      layer which can occur at the WSPConnect and ConnectEx functions. This method
//      determines whether the application's destination address should be
//      redirected to another destination. Currently, this function just looks for 
//      a single IPv4 destination address and substitutes it with another.
//
void
FindDestinationAddress( 
        SOCKET_CONTEXT *context, 
        const SOCKADDR *destAddr, 
        int             destLen,
        SOCKADDR      **proxyAddr, 
        int            *proxyLen
        )
{
    UNREFERENCED_PARAMETER( context );
    UNREFERENCED_PARAMETER( destLen );
    UNREFERENCED_PARAMETER( proxyAddr );
    UNREFERENCED_PARAMETER( proxyLen );

    context->AddressLength = destLen;

    // Save destination address
    memcpy( &context->OriginalAddress, destAddr, context->AddressLength );

    *proxyAddr = (SOCKADDR *) &context->OriginalAddress;
    *proxyLen  = context->AddressLength;

    if ( destAddr->sa_family == AF_INET )
    {
        // Redirect one destination to another
        if ( ( (SOCKADDR_IN *)destAddr )->sin_addr.s_addr == inet_addr("157.56.236.201") )
        {
            memcpy( &context->ProxiedAddress, destAddr, context->AddressLength );
            ( (SOCKADDR_IN *)&context->ProxiedAddress )->sin_addr.s_addr = inet_addr(
                    "157.56.237.9"
                    );

            *proxyAddr = (SOCKADDR *) &context->ProxiedAddress;

            context->Proxied = TRUE;
        }
    }
    else if ( destAddr->sa_family == AF_INET6 )
    {
        // Perform redirection here
    }
}

//
// Function: FreeLspProviders
//
// Description:
//
//
void
FreeLspProviders(
        PROVIDER   *lspProvider,
        int         lspProviderCount,
        int        *lpErrno
        )
{
    int     i;

    if ( NULL == lspProvider )
        return;

    // Need to iterate through the LSP providers and call WSPCleanup accordingly
    for(i=0; i < lspProviderCount ;i++)
    {
        while( 0 != lspProvider[ i ].StartupCount )
        {
            lspProvider[ i ].StartupCount--;

            lspProvider[ i ].NextProcTable.lpWSPCleanup( lpErrno );
        }

        if ( NULL != lspProvider[ i ].Module )
        {
            FreeLibrary( lspProvider[ i ].Module );
            lspProvider[ i ].Module = NULL;
        }
    }

    for(i=0; i < lspProviderCount ;i++)
    {
        FreeSocketContextList( &lspProvider[ i ] );
        
        DeleteCriticalSection( &lspProvider[ i ].ProviderCritSec );
    }

    LspFree( lspProvider );
}

//
// Function: FindUrl
//
// Description:
//      This routine searches each send buffer for the presence of an HTPT GET
//      request. It parses the buffer and returns the URL which is being requested.
//
int
FindUrl( 
        __in_ecount(buflen) char   *buf,
        int     buflen,
        __out char  **start
        )
{
    char   *subptr = NULL, *substart = NULL;
    int     subidx,
            idx;

    *start = NULL;

    // Perform a substring search starting at the beginning of 'buf'
    idx = 0;
    while ( idx < buflen )
    {
        substart = buf;
        subidx   = idx;
        subptr   = "GET";

        // Once a character is matched proceed to check for a complete match 
        //    while ensuring we don't go past the end of the buffer (since it may not
        //    be NULL terminated -- at least not the portion we're currently looking
        //    at)
        while ( ( subidx < buflen ) && ( *subptr != '\0' ) && ( *subptr == *substart ) )
        {
            subptr++;
            substart++;
            subidx++;
        }

        // If 'subptr' is pointing to NULL then a match to "GET" was found
        if ( *subptr == '\0' )
        {
            // A GET request was found so skip over the subsequent space
            idx = subidx + 1;
            
            // Validate we're still within the buffer
            if ( idx >= buflen )
            {
                // Went past the buffer so return no match
                return 0;
            }

            *start = substart + 1;  // Advance the character pointer past the space

            substart++;
            subidx = idx;

            // Advance the search pointer until we hit the end space
            while ( ( subidx < buflen ) && ( *substart != ' ' ) )
            {
                substart++;
                subidx++;
            }

            // If we're sitting on the trailing space, we found a URL so return
            //    how many bytes are in the URL
            if ( *substart == ' ' )
            {
                return subidx - idx;
            }
            else
            {
                // Error occured so just return since we're not where we expected
                return 0;
            }
        }

        // Advance the main pointers and index and start the search with the next
        //    string location
        buf++;
        idx++;
    }

    return 0;
}
