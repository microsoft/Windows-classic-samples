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
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through an IP socket. The application
//    reports when sockets are created and reports how many bytes were sent and
//    received when a socket closes. The results are reported using the OutputDebugString
//    API which will allow you to intercept the I/O by using a debugger such as cdb.exe
//    or you can monitor the I/O using dbmon.exe.
//
//    This file contains the 30 SPI functions you are required to implement in a
//    service provider. It also contains the two functions that must be exported
//    from the DLL module DllMain and WSPStartup.
//    

#include "lspdef.h"

#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable:4127)       // Disable "conditional expression is constant" warning

////////////////////////////////////////////////////////////////////////////////
//
// Globals used across files
//
////////////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION    gCriticalSection,   // Critical section for initialization and socket list
                    gOverlappedCS;      // Critical section for overlapped manager
WSPUPCALLTABLE      gMainUpCallTable;   // Winsock upcall table
HINSTANCE           gDllInstance = NULL;// DLL instance handle
LPPROVIDER          gBaseInfo = NULL;   // Provider information for each layer under us
INT                 gLayerCount = 0;    // Number of base providers we're layered over
HANDLE              gAddContextEvent=NULL;  // Event to set when adding socket context

////////////////////////////////////////////////////////////////////////////////
//
// Macros and Function Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Close all open sockets and free any associated resources
void 
FreeSocketsAndMemory(
    BOOL processDetach,
    int *lpErrno
    );

//
// Need to keep track of which PROVIDERs that are currently executing
//  a blocking Winsock call on a per thread basis.
//
#define SetBlockingProvider(Provider)           \
    ( gTlsIndex!=0xFFFFFFFF )                   \
        ? TlsSetValue ( gTlsIndex, Provider )   \
        : NULL

////////////////////////////////////////////////////////////////////////////////
//
// Globals local to this file
//
////////////////////////////////////////////////////////////////////////////////

static DWORD    gTlsIndex = 0xFFFFFFFF; // Index into thread local storage
static DWORD    gEntryCount = 0;        // How many times WSPStartup has been called
static DWORD    gLayerCatId = 0;        // Catalog ID of our dummy entry
static WSPDATA  gWSPData;
static BOOL     gDetached = FALSE;      // Indicates if process is detaching from DLL

// Fill out our proc table with our own LSP functions
static WSPPROC_TABLE       gProcTable = {
    WSPAccept,
    WSPAddressToString,
    WSPAsyncSelect,
    WSPBind,
    WSPCancelBlockingCall,
    WSPCleanup,
    WSPCloseSocket,
    WSPConnect,
    WSPDuplicateSocket,
    WSPEnumNetworkEvents,
    WSPEventSelect,
    WSPGetOverlappedResult,
    WSPGetPeerName,
    WSPGetSockName,
    WSPGetSockOpt,
    WSPGetQOSByName,
    WSPIoctl,
    WSPJoinLeaf,
    WSPListen,
    WSPRecv,
    WSPRecvDisconnect,
    WSPRecvFrom,
    WSPSelect,
    WSPSend,
    WSPSendDisconnect,
    WSPSendTo,
    WSPSetSockOpt,
    WSPShutdown,
    WSPSocket,
    WSPStringToAddress
    };

////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: PrintProcTable
//
// Description
//    Print the table of function pointers. This can be useful in tracking
//    down bugs with other LSP being layered over.
//
void 
PrintProcTable(
    LPWSPPROC_TABLE lpProcTable
    )
{
    #ifdef DBG_PRINTPROCTABLE
    dbgprint("WSPAccept              = 0x%X", lpProcTable->lpWSPAccept);
    dbgprint("WSPAddressToString     = 0x%X", lpProcTable->lpWSPAddressToString);
    dbgprint("WSPAsyncSelect         = 0x%X", lpProcTable->lpWSPAsyncSelect);
    dbgprint("WSPBind                = 0x%X", lpProcTable->lpWSPBind);
    dbgprint("WSPCancelBlockingCall  = 0x%X", lpProcTable->lpWSPCancelBlockingCall);
    dbgprint("WSPCleanup             = 0x%X", lpProcTable->lpWSPCleanup);
    dbgprint("WSPCloseSocket         = 0x%X", lpProcTable->lpWSPCloseSocket);
    dbgprint("WSPConnect             = 0x%X", lpProcTable->lpWSPConnect);
    dbgprint("WSPDuplicateSocket     = 0x%X", lpProcTable->lpWSPDuplicateSocket);
    dbgprint("WSPEnumNetworkEvents   = 0x%X", lpProcTable->lpWSPEnumNetworkEvents);
    dbgprint("WSPEventSelect         = 0x%X", lpProcTable->lpWSPEventSelect);
    dbgprint("WSPGetOverlappedResult = 0x%X", lpProcTable->lpWSPGetOverlappedResult);
    dbgprint("WSPGetPeerName         = 0x%X", lpProcTable->lpWSPGetPeerName);
    dbgprint("WSPGetSockOpt          = 0x%X", lpProcTable->lpWSPGetSockOpt);
    dbgprint("WSPGetSockName         = 0x%X", lpProcTable->lpWSPGetSockName);
    dbgprint("WSPGetQOSByName        = 0x%X", lpProcTable->lpWSPGetQOSByName);
    dbgprint("WSPIoctl               = 0x%X", lpProcTable->lpWSPIoctl);
    dbgprint("WSPJoinLeaf            = 0x%X", lpProcTable->lpWSPJoinLeaf);
    dbgprint("WSPListen              = 0x%X", lpProcTable->lpWSPListen);
    dbgprint("WSPRecv                = 0x%X", lpProcTable->lpWSPRecv);
    dbgprint("WSPRecvDisconnect      = 0x%X", lpProcTable->lpWSPRecvDisconnect);
    dbgprint("WSPRecvFrom            = 0x%X", lpProcTable->lpWSPRecvFrom);
    dbgprint("WSPSelect              = 0x%X", lpProcTable->lpWSPSelect);
    dbgprint("WSPSend                = 0x%X", lpProcTable->lpWSPSend);
    dbgprint("WSPSendDisconnect      = 0x%X", lpProcTable->lpWSPSendDisconnect);
    dbgprint("WSPSendTo              = 0x%X", lpProcTable->lpWSPSendTo);
    dbgprint("WSPSetSockOpt          = 0x%X", lpProcTable->lpWSPSetSockOpt);
    dbgprint("WSPShutdown            = 0x%X", lpProcTable->lpWSPShutdown);
    dbgprint("WSPSocket              = 0x%X", lpProcTable->lpWSPSocket);
    dbgprint("WSPStringToAddress     = 0x%X", lpProcTable->lpWSPStringToAddress);
    #else
    UNREFERENCED_PARAMETER( lpProcTable );  // For W4 compliance
    #endif
}

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
    switch (dwReason)
    {

        case DLL_PROCESS_ATTACH:
            gDllInstance = hinstDll;
            //
            // Initialize some critical section objects 
            //
            __try
            {
                InitializeCriticalSection( &gCriticalSection );
                InitializeCriticalSection( &gOverlappedCS );
                InitializeCriticalSection( &gDebugCritSec );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                goto cleanup;
            }

            gTlsIndex = TlsAlloc();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            gDetached = TRUE;

            EnterCriticalSection( &gCriticalSection );
            if ( NULL != gBaseInfo )
            {
                int Error;

                StopAsyncWindowManager();
                StopOverlappedManager();

                Sleep(200);

                FreeSocketsAndMemory( TRUE, &Error );
            }
            LeaveCriticalSection( &gCriticalSection );

            DeleteCriticalSection( &gCriticalSection );
            DeleteCriticalSection( &gOverlappedCS );
            DeleteCriticalSection( &gDebugCritSec );

            if ( NULL == lpvReserved )
            {
                if ( 0xFFFFFFFF != gTlsIndex )
                {
                    TlsFree( gTlsIndex );
                    gTlsIndex = 0xFFFFFFFF;
                }
            }
            break;
    }

    return TRUE;

cleanup:

    return FALSE;
}

//
// Function: WSPAccept
//
// Description:
//    Handle the WSAAccept function. The only special consideration here is the
//    conditional accept callback. You can choose to intercept this by substituting
//    your own callback (you'll need to keep track of the user supplied callback so
//    you can trigger that once your substituted function is triggered).
//
SOCKET WSPAPI 
WSPAccept(
    SOCKET          s,                      
    struct sockaddr FAR * addr,  
    LPINT           addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD_PTR       dwCallbackData,          
    LPINT           lpErrno
    )
{
    SOCKET     NewProviderSocket;
    SOCKET     NewSocket = INVALID_SOCKET;
    SOCK_INFO *NewSocketContext = NULL;
    SOCK_INFO *SocketContext = NULL;

    //
    // Query for our per socket info
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPAccept: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    //
    // Note: You can subsitute your own conditional accept callback function
    //       in order to intercept this callback. You would have to keep track
    //       of the user's callback function so that you can call that when
    //       your intermediate function executes.
    //

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPAccept );

    SetBlockingProvider(SocketContext->Provider);
    NewProviderSocket = SocketContext->Provider->NextProcTable.lpWSPAccept(
                            SocketContext->ProviderSocket, 
                            addr, 
                            addrlen,
                            lpfnCondition, 
                            dwCallbackData, 
                            lpErrno);
    SetBlockingProvider(NULL);
    if ( INVALID_SOCKET != NewProviderSocket )
    {
        // The underlying provider received a new connection so lets create our own
        //  socket to pass back up to the application.
        //
        NewSocketContext = CreateSockInfo(
                SocketContext->Provider,
                NewProviderSocket,
                SocketContext,
                FALSE,
                lpErrno
                );
        if  ( NULL == NewSocketContext )
        {
            goto cleanup;
        }
        
        NewSocket = NewSocketContext->LayeredSocket = gMainUpCallTable.lpWPUCreateSocketHandle(
                SocketContext->Provider->LayerProvider.dwCatalogEntryId,
                (DWORD_PTR) NewSocketContext,
                lpErrno);
        if ( INVALID_SOCKET == NewSocket )
        {
            int     tempErr;

            dbgprint("WSPAccept(): WPUCreateSocketHandle() failed: %d", *lpErrno);
            
            // Close the lower provider's socket but preserve the original error value
            SocketContext->Provider->NextProcTable.lpWSPCloseSocket(
                NewProviderSocket,
               &tempErr
                );

            // Context is not in the list yet so we can just free it
            FreeSockInfo(NewSocketContext);
        }
        else
        {
            InsertSocketInfo(SocketContext->Provider, NewSocketContext);
        }
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return NewSocket;
}

//
// Function: WSPAdressToString
//
// Description:
//    Convert an address to string. We simply pass this to the lower provider.
//
int WSPAPI 
WSPAddressToString(
    LPSOCKADDR          lpsaAddress,            
    DWORD               dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR              lpszAddressString,            
    LPDWORD             lpdwAddressStringLength,   
    LPINT               lpErrno
    )
{
    WSAPROTOCOL_INFOW *pInfo=NULL;
    PROVIDER          *Provider=NULL;
    INT                ret = SOCKET_ERROR,
                       i;

    //
    // First find the appropriate provider
    //
    for(i=0; i < gLayerCount ;i++)
    {
        if ((gBaseInfo[i].NextProvider.iAddressFamily == lpProtocolInfo->iAddressFamily) &&
            (gBaseInfo[i].NextProvider.iSocketType == lpProtocolInfo->iSocketType) && 
            (gBaseInfo[i].NextProvider.iProtocol   == lpProtocolInfo->iProtocol))
        {
            if ( NULL != lpProtocolInfo )
            {
                // In case of multiple providers check the provider flags 
                if ( ( gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES ) != 
                     ( lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES ) 
                   )
                {
                    continue;
                }
            }
            Provider = &gBaseInfo[i];
            pInfo = &gBaseInfo[i].NextProvider;
            break;
        }
    }

    if ( NULL == Provider )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // Of course if the next layer isn't a base just pass down lpProtocolInfo.
    //
    if ( BASE_PROTOCOL != pInfo->ProtocolChain.ChainLen )
    {
        pInfo = lpProtocolInfo;
    }
   
    if ( 0 == Provider->StartupCount )
    {
        if ( SOCKET_ERROR == InitializeProvider( Provider, MAKEWORD(2,2), lpProtocolInfo,
                gMainUpCallTable, lpErrno ) )
        {
            dbgprint("WSPAddressToString: InitializeProvider failed: %d", *lpErrno);
            goto cleanup;
        }
    }

    ASSERT( Provider->NextProcTable.lpWSPAddressToString );

    SetBlockingProvider(Provider);
    ret = Provider->NextProcTable.lpWSPAddressToString(
            lpsaAddress, 
            dwAddressLength,               
            pInfo, 
            lpszAddressString, 
            lpdwAddressStringLength, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    return ret;
}

//
// Function: WSPAsyncSelect
//
// Description:
//    Register specific Winsock events with a socket. We need to substitute
//    the app socket with the provider socket and use our own hidden window.
//
int WSPAPI 
WSPAsyncSelect(
    SOCKET       s,
    HWND         hWnd,
    unsigned int wMsg,
    long         lEvent,
    LPINT        lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    HWND       hWorkerWindow = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Make sure the window handle is valid
    //
    ret = SOCKET_ERROR;
    if ( FALSE == IsWindow( hWnd ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // Verify only valid events have been set
    //
    if ( 0 != (lEvent & ~FD_ALL_EVENTS) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPAsyncSelect: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    SocketContext->hWnd = hWnd;
    SocketContext->uMsg = wMsg;

    //
    // Get the handle to our hidden window
    //
    if ( NULL == ( hWorkerWindow = GetWorkerWindow() ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPAsyncSelect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPAsyncSelect(
            SocketContext->ProviderSocket, 
            hWorkerWindow, 
            WM_SOCKET, 
            lEvent, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPBind
//
// Description:
//    Bind the socket to a local address. We just map socket handles and
//    call the lower provider.
//
int WSPAPI 
WSPBind(
    SOCKET                s,
    const struct sockaddr FAR * name,
    int                   namelen,
    LPINT                 lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPBind: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPBind );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPBind(
            SocketContext->ProviderSocket, 
            name, 
            namelen, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPCancelBlockingCall
//
// Description:
//    This call cancels any blocking Winsock call in the current thread only.
//    For every Winsock call that blocks we use thread local storage (TLS) to
//    store a pointer to the provider on which the blocking call was issued.
//    This is necessary since WSACancelBlockingCall takes no arguments (i.e.
//    the LSP needs to keep track of what calls are blocking).
//
int WSPAPI 
WSPCancelBlockingCall(
    LPINT lpErrno
    )
{
    PROVIDER *Provider = NULL;
    INT       ret = NO_ERROR;

    Provider = (PROVIDER *) TlsGetValue( gTlsIndex );
    if ( NULL != Provider )
    {
        ASSERT( Provider->NextProcTable.lpWSPCancelBlockingCall );

        ret = Provider->NextProcTable.lpWSPCancelBlockingCall(lpErrno);
    }
    return ret;
}

// 
// Function: WSPCleanup
//
// Description:
//    Decrement the entry count. If equal to zero then we can prepare to have us
//    unloaded. Close any outstanding sockets and free up allocated memory.
//
int WSPAPI 
WSPCleanup(
    LPINT lpErrno  
    )
{
    int        ret = SOCKET_ERROR;

    if ( gDetached )
    {
        dbgprint("WSPCleanup: DLL has already been unloaded from process!");
        ret = NO_ERROR;
        return ret;
    }

    //
    // Grab the DLL global critical section
    //
    EnterCriticalSection( &gCriticalSection );

    if ( 0 == gEntryCount )
    {
        *lpErrno = WSANOTINITIALISED;
        dbgprint("WSPCleanup returning WSAENOTINITIALISED");
        goto cleanup;
    }

    //
    // Decrement the entry count
    //
    gEntryCount--;

    #ifdef DEBUG
    dbgprint("WSPCleanup: %d", gEntryCount);
    #endif

    if ( 0 == gEntryCount )
    {
        //
        // App released the last reference to use so shutdown the async window
        // and overlapped threads.
        //
        StopAsyncWindowManager();
        StopOverlappedManager();

        Sleep(200);

        FreeOverlappedLookasideList();

        FreeSocketsAndMemory( FALSE, lpErrno );
    }

cleanup:

    LeaveCriticalSection(&gCriticalSection);

    return ret;
}

//
// Function: WSPCloseSocket
//
// Description:
//    Close the socket handle of the app socket as well as the provider socket.
//    However, if there are outstanding async IO requests on the app socket
//    we only close the provider socket. Only when all the IO requests complete
//    (with error) will we then close the app socket (this will occur in
//    the overlapped manager - overlapp.cpp).
//
int WSPAPI 
WSPCloseSocket(  
    SOCKET s,        
    LPINT  lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    int        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPCloseSocket: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    AcquireSocketLock( SocketContext );

    dbgprint("WSPCloseSocket: Closing layered socket 0x%p (provider 0x%p)",
        s, SocketContext->ProviderSocket);

    //
    // If we there are outstanding async calls on this handle don't close the app
    //  socket handle...only close the provider's handle.  Therefore any errors
    //  incurred can be propogated back to the app socket. Also verify closesocket
    //  hasn't already been called on this socket
    //

    dbgprint("dwOutstanding = %d; RefCount = %d", SocketContext->dwOutstandingAsync, 
        SocketContext->RefCount);

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPCloseSocket );

    if ( ( ( 0 != SocketContext->dwOutstandingAsync ) || 
           ( 1 != SocketContext->RefCount ) ) &&
         ( TRUE != SocketContext->bClosing )
       )
    {
        //
        // Either there are outstanding asynchronous operations or some other thread
        // is performing an operation AND close has not already been called on this
        // socket
        //
        SocketContext->bClosing = TRUE;

        ret = SocketContext->Provider->NextProcTable.lpWSPCloseSocket(
                SocketContext->ProviderSocket, 
                lpErrno
                );
        if ( SOCKET_ERROR == ret )
        {
            goto cleanup;
        }
       
        dbgprint("Closed lower provider socket: 0x%p", SocketContext->ProviderSocket);

        SocketContext->ProviderSocket = INVALID_SOCKET;

    }
    else if ( ( 0 == SocketContext->dwOutstandingAsync ) &&
              ( 1 == SocketContext->RefCount )
            )
    {
        //
        // No one else is referencing this socket so we can close and free all
        //  objects associated with it
        //
        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPCloseSocket(
                SocketContext->ProviderSocket, 
                lpErrno
                );
        
        SetBlockingProvider(NULL);
        
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WSPCloseSocket: Provider close failed");
            goto cleanup;
        }
        

        SocketContext->ProviderSocket = INVALID_SOCKET;

        //
        // Remove the socket info
        //
        RemoveSocketInfo(SocketContext->Provider, SocketContext);

        //
        // Close the app socket
        //
        ret = gMainUpCallTable.lpWPUCloseSocketHandle(s, lpErrno);
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WPUCloseSocketHandle failed: %d", *lpErrno);
        }

        dbgprint("Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]", 
                s, SocketContext->BytesSent, SocketContext->BytesRecv);

        ReleaseSocketLock( SocketContext );

        // Don't need to 'DerefSocketContext' as we're deleting the object now

        FreeSockInfo( SocketContext );
        SocketContext = NULL;
    }

cleanup:
    
    if ( NULL != SocketContext )
    {
        ReleaseSocketLock(SocketContext);
        DerefSocketContext( SocketContext, lpErrno );
    }

    return ret;
}

//
// Function: WSPConnect
//
// Description:
//    Performs a connect call. The only thing we need to do is translate
//    the socket handle.
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
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPConnect: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPConnect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPConnect(
            SocketContext->ProviderSocket, 
            name, 
            namelen, 
            lpCallerData, 
            lpCalleeData,
            lpSQOS, 
            lpGQOS, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPDuplicateSocket
//
// Description:
//    This function provides a WSAPROTOCOL_INFOW structure which can be passed
//    to another process to open a handle to the same socket. First we need
//    to translate the user socket into the provider socket and call the underlying
//    WSPDuplicateSocket. Note that the lpProtocolInfo structure passed into us
//    is an out parameter only!
//
int WSPAPI 
WSPDuplicateSocket(
    SOCKET              s,
    DWORD               dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT               lpErrno
    )
{
    PROVIDER          *Provider = NULL;
    SOCK_INFO         *SocketContext = NULL;
    DWORD              dwReserved;
    int                ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPDuplicateSocket: FindAndRefSocketContext failed!" );
        goto cleanup;
    }
    //
    // Find the underlying provider
    //
    Provider = SocketContext->Provider;

    ASSERT( Provider->NextProcTable.lpWSPDuplicateSocket );

    SetBlockingProvider(Provider);
    ret = Provider->NextProcTable.lpWSPDuplicateSocket(
            SocketContext->ProviderSocket,
            dwProcessId,
            lpProtocolInfo,
            lpErrno
            );
    SetBlockingProvider(NULL);

    if ( NO_ERROR == ret )
    {
        //
        // We want to return the WSAPROTOCOL_INFOW structure of the underlying
        // provider but we need to preserve the reserved info returned by the
        // WSPDuplicateSocket call.
        //
        dwReserved = lpProtocolInfo->dwProviderReserved;
        memcpy(lpProtocolInfo, &Provider->LayerProvider, sizeof(WSAPROTOCOL_INFOW));
        lpProtocolInfo->dwProviderReserved = dwReserved;

        dbgprint("WSPDuplicateSocket: Returning %S provider with reserved %d",
                lpProtocolInfo->szProtocol, dwReserved );
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;    
}

//
// Function: WSPEnumNetworkEvents
//
// Description:
//    Enumerate the network events for a socket. We only need to translate the
//    socket handle.
//
int WSPAPI 
WSPEnumNetworkEvents(  
    SOCKET             s,
    WSAEVENT           hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT              lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPEnumNetworkEvents: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPEnumNetworkEvents );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPEnumNetworkEvents(
            SocketContext->ProviderSocket,                             
            hEventObject, 
            lpNetworkEvents, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPEventSelect
//
// Description:
//    Register the specified events on the socket with the given event handle.
//    All we need to do is translate the socket handle.
//
int WSPAPI 
WSPEventSelect(
    SOCKET   s,
    WSAEVENT hEventObject,
    long     lNetworkEvents,
    LPINT    lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPEventSelect: FindAndRefSocketContext failed!" );
        goto cleanup;
    }
    
    ASSERT( SocketContext->Provider->NextProcTable.lpWSPEventSelect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPEventSelect(
            SocketContext->ProviderSocket, 
            hEventObject,
            lNetworkEvents, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPGetOverlappedResult
//
// Description:
//    This function reports whether the specified overlapped call has
//    completed. If it has, return the requested information. If not,
//    and fWait is true, wait until completion. Otherwise return an
//    error immediately.
//
BOOL WSPAPI 
WSPGetOverlappedResult(
    SOCKET          s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD         lpcbTransfer,
    BOOL            fWait,
    LPDWORD         lpdwFlags,
    LPINT           lpErrno
    )
{
    DWORD ret = FALSE;

    UNREFERENCED_PARAMETER( s );

    __try
    {
        if ( WSS_OPERATION_IN_PROGRESS != lpOverlapped->Internal ) 
        {
            // Operation has completed, update the parameters and return 
            //
            *lpcbTransfer = (DWORD)lpOverlapped->InternalHigh;
            *lpdwFlags = (DWORD)lpOverlapped->Offset;
            *lpErrno = (INT)lpOverlapped->OffsetHigh;

            ret = (lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
        }
        else if ( FALSE != fWait )
        {
            //
            // Operation is still in progress so wait until it completes
            //

            //
            // Wait on the app supplied event handle. Once the operation
            //  is completed the IOCP or completion routine will fire.
            //  Once that is handled, WPUCompleteOverlappedRequest will
            //  be called which will signal the app event.
            //
            ret = WaitForSingleObject(lpOverlapped->hEvent, INFINITE);
            if ( ( WAIT_OBJECT_0 == ret ) &&
                    ( WSS_OPERATION_IN_PROGRESS != lpOverlapped->Internal ) )
            {
                *lpcbTransfer = (DWORD)lpOverlapped->InternalHigh;
                *lpdwFlags = (DWORD)lpOverlapped->Offset;
                *lpErrno = (INT)lpOverlapped->OffsetHigh;

                ret = (lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
            }
            else if ( WSS_OPERATION_IN_PROGRESS == lpOverlapped->Internal )
            {
                *lpErrno = WSA_IO_PENDING;
            }
            else 
            {
                *lpErrno = GetLastError();
            }
        }
        else 
        {
            // Operation is in progress and we aren't waiting
            *lpErrno = WSA_IO_INCOMPLETE;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        *lpErrno = WSAEFAULT;
    }

    return ret;
}

//
// Function: WSPGetPeerName
//
// Description:
//    Returns the address of the peer. The only thing we need to do is translate
//    the socket handle.
//
int WSPAPI 
WSPGetPeerName(  
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPGetPeerName: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPGetPeerName );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPGetPeerName(
                SocketContext->ProviderSocket, 
                name,
                namelen, 
                lpErrno);
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPGetSockName
//
// Description:
//    Returns the local address of a socket. All we need to do is translate
//    the socket handle.
//
int WSPAPI 
WSPGetSockName(
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPGetSockName: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPGetSockName );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPGetSockName(
            SocketContext->ProviderSocket, 
            name,
            namelen, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPGetSockOpt
//
// Description:
//    Get the specified socket option. All we need to do is translate the
//    socket handle.
//
int WSPAPI 
WSPGetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    char FAR * optval,
    LPINT      optlen,
    LPINT      lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = NO_ERROR;


    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPGetSockOpt: FindAndRefSocketContext failed!" );
        goto cleanup;
    }


    __try
    {
        //
        // We need to capture this and return our own WSAPROTOCOL_INFO structure.
        // Otherwise, if we translate the handle and pass it to the lower provider
        // we'll return the lower provider's protocol info!
        //
        if ( ( SOL_SOCKET == level ) && ( ( SO_PROTOCOL_INFOA == optname ) ||
                                          ( SO_PROTOCOL_INFOW == optname ) )
           )
        {
            if ( ( SO_PROTOCOL_INFOW == optname ) && 
                 ( sizeof( WSAPROTOCOL_INFOW ) <= *optlen )
               )
            {

                    // No conversion necessary, just copy the data
                    memcpy(optval, 
                           &SocketContext->Provider->LayerProvider, 
                           sizeof(WSAPROTOCOL_INFOW));
       
            }
            else if ( ( SO_PROTOCOL_INFOA == optname ) && 
                      ( sizeof( WSAPROTOCOL_INFOA ) <= *optlen )
                    )
            {
             
                // Copy everything but the string
                memcpy(optval,
                       &SocketContext->Provider->LayerProvider,
                       sizeof(WSAPROTOCOL_INFOW)-WSAPROTOCOL_LEN+1);
                // Convert our saved UNICODE string to ANSII
                WideCharToMultiByte(
                        CP_ACP,
                        0,
                        SocketContext->Provider->LayerProvider.szProtocol,
                        -1,
                        ((WSAPROTOCOL_INFOA *)optval)->szProtocol,
                        WSAPROTOCOL_LEN+1,
                        NULL,
                        NULL
                        );

     
            }
            else
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
                goto cleanup;
            }

            goto cleanup;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        ret = SOCKET_ERROR;
        *lpErrno = WSAEFAULT;
        goto cleanup;
    }       
   
    ASSERT( SocketContext->Provider->NextProcTable.lpWSPGetSockOpt );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPGetSockOpt(
                SocketContext->ProviderSocket, 
                level,
                optname, 
                optval, 
                optlen, 
                lpErrno);
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPGetQOSByName
//
// Description:
//    Get a QOS template by name. All we need to do is translate the socket
//    handle.
//
BOOL WSPAPI 
WSPGetQOSByName(
    SOCKET   s,
    LPWSABUF lpQOSName,
    LPQOS    lpQOS,
    LPINT    lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPGetQOSByName: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPGetQOSByName );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPGetQOSByName(
            SocketContext->ProviderSocket, 
            lpQOSName,
            lpQOS, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPIoctl
//
// Description:
//    Invoke an ioctl. In most cases, we just need to translate the socket
//    handle. However, if the dwIoControlCode is SIO_GET_EXTENSION_FUNCTION_POINTER,
//    we'll need to intercept this and return our own function pointers when
//    they're requesting either TransmitFile or AcceptEx. This is necessary so
//    we can trap these calls. Also for PnP OS's (Win2k) we need to trap calls
//    to SIO_QUERY_TARGET_PNP_HANDLE. For this ioctl we simply have to return 
//    the provider socket.
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
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    SOCK_INFO          *SocketContext = NULL;
    GUID                AcceptExGuid = WSAID_ACCEPTEX,
                        TransmitFileGuid = WSAID_TRANSMITFILE,
                        GetAcceptExSockAddrsGuid = WSAID_GETACCEPTEXSOCKADDRS,
                        ConnectExGuid = WSAID_CONNECTEX,
                        DisconnectExGuid = WSAID_DISCONNECTEX,
                        TransmitPacketsGuid = WSAID_TRANSMITPACKETS,
                        WSARecvMsgGuid = WSAID_WSARECVMSG;
    DWORD               dwBytesReturned = 0;
    int                 ret = NO_ERROR;


    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPIoctl: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    if ( SIO_GET_EXTENSION_FUNCTION_POINTER == dwIoControlCode )
    {
        LPVOID      lpFunction = NULL;


        // Sanity check the buffers
        
        if( cbInBuffer < sizeof(GUID) || lpvInBuffer == NULL ||
            cbOutBuffer < sizeof(LPVOID) || lpvOutBuffer == NULL )        
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;            
        }
        
        __try
        {
            //
            // Check to see which extension function is being requested.
            //
            if ( 0 == memcmp( lpvInBuffer, &TransmitFileGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extesion function
                dwBytesReturned = sizeof(LPFN_TRANSMITFILE);
                lpFunction = ExtTransmitFile;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnTransmitFile )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnTransmitFile,
                            TransmitFileGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &AcceptExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_ACCEPTEX );
                lpFunction = ExtAcceptEx;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnAcceptEx )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnAcceptEx,
                            AcceptExGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &ConnectExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_CONNECTEX );
                lpFunction = ExtConnectEx;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnConnectEx )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnConnectEx,
                            ConnectExGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &DisconnectExGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_DISCONNECTEX );
                lpFunction = ExtDisconnectEx;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnDisconnectEx )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnDisconnectEx,
                            DisconnectExGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &TransmitPacketsGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_TRANSMITPACKETS );
                lpFunction = ExtTransmitPackets;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnTransmitPackets )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnTransmitPackets,
                            TransmitPacketsGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &WSARecvMsgGuid, sizeof( GUID ) ) )
            {
                // Return a pointer to our intermediate extension function
                dwBytesReturned = sizeof( LPFN_WSARECVMSG );
                lpFunction = ExtWSARecvMsg;

                // Attempt to load the lower provider's extension function
                if ( NULL == SocketContext->Provider->NextProcTableExt.lpfnWSARecvMsg )
                {
                    SetBlockingProvider(SocketContext->Provider);
                    LoadExtensionFunction(
                            (FARPROC **) &SocketContext->Provider->NextProcTableExt.lpfnWSARecvMsg,
                            WSARecvMsgGuid,
                            SocketContext->Provider->NextProcTable.lpWSPIoctl,
                            SocketContext->ProviderSocket
                            );
                    SetBlockingProvider(NULL);
                }
            }
            else if ( 0 == memcmp( lpvInBuffer, &GetAcceptExSockAddrsGuid, sizeof( GUID ) ) )
            {
                // No socket handle translation needed, let the call pass through below
                // (i.e. we really don't have any need to intercept this call)
            }
            else 
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEINVAL;
                goto cleanup;
            }
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;
                    
        }
        //
        // Update the output parameters if successful
        //
        if ( NULL != lpFunction )
        {
            __try 
            {
                *((DWORD_PTR *)lpvOutBuffer) = (DWORD_PTR) lpFunction;
                *lpcbBytesReturned = dwBytesReturned;
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
            }
            goto cleanup;
        }

        // Only if GetAcceptExSockAddresGuid was passed in will we get here
        // We fall through and make the call to the lower layer

    }
    else if ( SIO_QUERY_TARGET_PNP_HANDLE == dwIoControlCode )
    {
        //
        // If the next layer is another LSP, keep passing. Otherwise return the 
        //    lower provider's handle so it may be used in PNP event notifications.
        //
        if ( SocketContext->Provider->NextProvider.ProtocolChain.ChainLen != BASE_PROTOCOL )
        {
            __try
            {
                *((SOCKET *)lpvOutBuffer) = SocketContext->ProviderSocket;
                dwBytesReturned = *lpcbBytesReturned = sizeof(SocketContext->ProviderSocket);
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                ret = SOCKET_ERROR;
                *lpErrno = WSAEFAULT;
                goto cleanup;
            }

            if ( NULL != lpOverlapped )
            {
                ProviderOverlapped = PrepareOverlappedOperation(
                        SocketContext,
                        LSP_OP_IOCTL,
                        NULL,
                        0,
                        lpOverlapped,
                        lpCompletionRoutine,
                        lpThreadId,
                        lpErrno
                        );
                if ( NULL == ProviderOverlapped )
                    goto cleanup;

                __try
                {
                    ProviderOverlapped->IoctlArgs.cbBytesReturned   = (lpcbBytesReturned ? *lpcbBytesReturned : 0);
                }
                __except( EXCEPTION_EXECUTE_HANDLER )
                {
                    ret = SOCKET_ERROR;
                    *lpErrno = WSAEFAULT;
                    goto cleanup;
                }

                //
                // Call the completion routine immediately since there is nothing
                //  else to do. For this ioctl all we do is return the provider
                //  socket. If it was called overlapped just complete the operation.
                //

                dbgprint("SIO_QUERY_TARGET_PNP_HANDLE overlapped");

                IntermediateCompletionRoutine(
                        0,
                        dwBytesReturned,
                        (WSAOVERLAPPED *)ProviderOverlapped,
                        0);
            }

            goto cleanup;
        }
    }
    else if ( ( SIO_BSP_HANDLE == dwIoControlCode ) ||
              ( SIO_BSP_HANDLE_POLL == dwIoControlCode ) )

    {
        //
        // SIO_BSP_HANDLE needs to be intercepted if the LSP is to intercept the
        // WSASendMsg extension function on Vista. In addition to this ioctl,
        // the LSP must handle SIO_EXT_SENDMSG below.
        //
        // SIO_BSP_HANDLE_POLL needs to be intercepted if the LSP is to intercept the
        // WSAPoll extension function on Vista. In addition to this ioctl, the LSP
        // must handle SIO_EXT_POLL below.
        //
        // NOTE: If your LSP does not care about either of these functions it is safe
        //       to skip this code (both the SIO_BSP* and SIO_EXT* ioctl).
        //
        if ( NULL == lpvOutBuffer || cbOutBuffer < sizeof(SOCKET) )
        {
            *lpErrno = WSAEFAULT;
            ret = SOCKET_ERROR;
            goto cleanup;
        }

        //
        // Simply return the LSP created socket to Winock. This ensures that when the
        // application calls WSASendMsg and WSAPoll, that call will be routed to the
        // LSP for handling.
        //
        *(SOCKET *)lpvOutBuffer = s;
        goto cleanup;

    }
    else if ( SIO_EXT_SENDMSG == dwIoControlCode )
    {

        //
        // Intercept the WSASendMsg ioctl and redirect to the LSPs WSASendMsg function
        //

        if (cbInBuffer < sizeof(WSASENDMSG))
        {
            *lpErrno = WSAEFAULT;
            ret = SOCKET_ERROR;
            goto cleanup;
        }

        ret = ExtWSASendMsg(s, (WSASENDMSG *)lpvInBuffer, lpThreadId, lpErrno);
        if (SOCKET_ERROR == ret)
        {
            DWORD *errorCode  = (LPDWORD)lpvOutBuffer;

            *errorCode = *lpErrno;
        }

        goto cleanup;

    }
    else if ( SIO_EXT_POLL == dwIoControlCode )
    {
        WSAPOLLDATA *pollData;

        pollData = (WSAPOLLDATA *)lpvInBuffer;

        //
        // Intercept the WSAPoll ioctl and redirect to the LSPs WSAPoll function
        //

        if ( (cbInBuffer < sizeof(WSAPOLLDATA) ) ||
             (cbInBuffer < (sizeof(WSAPOLLDATA) + (pollData->fds * sizeof(WSAPOLLFD))) )
             )
        {
            *lpErrno = WSAEFAULT;
            ret = SOCKET_ERROR;
            goto cleanup;
        }

        ret = ExtWSAPoll(s, pollData, lpThreadId, lpErrno);
        if (SOCKET_ERROR == ret)
        {
            ret = SOCKET_ERROR;
            goto cleanup;
        }

        goto cleanup;

    }
    else if ( SIO_BSP_HANDLE_SELECT == dwIoControlCode )  
    {
        //
        // Winsock handles the select() function differently on Windows Vista.
        // If (and only if) an LSP is installed (whether yours or someone elses),
        // then Winsock will bypass ALL LSPs when the application calls select().
        //
        // This is done for system robustness. Many Vista components are both IPv4
        // and IPv6 enabled which means these applications pass both IPv4 and IPv6
        // socket handles to select(). If the LSP is only layered over IPv4 and if
        // the IPv6 socket is first in the FD_SET, the select call will go to the
        // provider that created the IPv6 socket (which could be a base provider).
        // In this case, the base provider cannot recongize the LSP owned IPv4 sockets
        // and will fail the call which will cause system critical services to fail.
        //
        // If your LSP needs to intercept select() calls you MUST do two things.
        // First you must uncomment the code below such that the LSP handle is returned,
        // and SECOND you must install your LSP over both IPv4 an IPv6. If you only
        // do the first, then it is likely that a different base provider will be
        // invoked to handle some select calls which will cause system instability.
        //

        /*

        if ( NULL == lpvOutBuffer || cbOutBuffer < sizeof(SOCKET) )
        {
            *lpErrno = WSAEFAULT;
            ret = SOCKET_ERROR;
            goto cleanup;
        }

        *(SOCKET *)lpvOutBuffer = s;
        goto cleanup;

        */

    }

    //
    // Check for overlapped I/O
    // 
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                SocketContext,
                LSP_OP_IOCTL,
                NULL,
                0,
                lpOverlapped,
                lpCompletionRoutine,
                lpThreadId,
                lpErrno
                );
        if ( NULL == ProviderOverlapped )
        {
            ret = SOCKET_ERROR;
            // lpErrno is set by PrepareOverlappedOperation
            goto cleanup;
        }

        __try
        {
            ProviderOverlapped->IoctlArgs.cbBytesReturned = (lpcbBytesReturned ? *lpcbBytesReturned : 0);
            ProviderOverlapped->IoctlArgs.dwIoControlCode = dwIoControlCode;
            ProviderOverlapped->IoctlArgs.lpvInBuffer     = lpvInBuffer;
            ProviderOverlapped->IoctlArgs.cbInBuffer      = cbInBuffer;
            ProviderOverlapped->IoctlArgs.lpvOutBuffer    = lpvOutBuffer;
            ProviderOverlapped->IoctlArgs.cbOutBuffer     = cbOutBuffer;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ret = SOCKET_ERROR;
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
        ret = QueueOverlappedOperation(ProviderOverlapped, SocketContext);
        if ( NO_ERROR != ret )
        {
            *lpErrno = ret;
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ASSERT( SocketContext->Provider->NextProcTable.lpWSPIoctl );

        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPIoctl(
                SocketContext->ProviderSocket, 
                dwIoControlCode, 
                lpvInBuffer,
                cbInBuffer, 
                lpvOutBuffer, 
                cbOutBuffer, 
                lpcbBytesReturned, 
                lpOverlapped, 
                lpCompletionRoutine, 
                lpThreadId, 
                lpErrno);
        SetBlockingProvider(NULL);
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPJoinLeaf
//
// Description:
//    This function joins a socket to a multipoint session. For those providers
//    that support multipoint semantics there are 2 possible behaviors. First,
//    for IP, WSAJoinLeaf always returns the same socket handle which was passed
//    into it. In this case there is no new socket so we don't want to create
//    any socket context once the lower provider WSPJoinLeaf is called. In the
//    second case, for ATM, a new socket IS created when we call the lower
//    provider. In this case we do want to create a new user socket and create
//    a socket context.
//
SOCKET WSPAPI 
WSPJoinLeaf(
    SOCKET       s,
    const struct sockaddr FAR * name,
    int          namelen,
    LPWSABUF     lpCallerData,
    LPWSABUF     lpCalleeData,
    LPQOS        lpSQOS,
    LPQOS        lpGQOS,
    DWORD        dwFlags,
    LPINT        lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    SOCKET     NextProviderSocket = INVALID_SOCKET,
               NewSocket = INVALID_SOCKET;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPJoinLeaf: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPJoinLeaf );

    SetBlockingProvider(SocketContext->Provider);
    NextProviderSocket = SocketContext->Provider->NextProcTable.lpWSPJoinLeaf(
            SocketContext->ProviderSocket,                           
            name, 
            namelen, 
            lpCallerData, 
            lpCalleeData, 
            lpSQOS, 
            lpGQOS, 
            dwFlags,                        
            lpErrno
            );
    SetBlockingProvider(NULL);

    //    
    // If the socket returned from the lower provider is the same as the socket
    //  passed into it then there really isn't a new socket - just return. 
    //  Otherwise, a new socket has been created and we need to create the socket
    //  context and create a user socket to pass back.
    //
    if ( ( INVALID_SOCKET != NextProviderSocket ) && 
         ( NextProviderSocket != SocketContext->ProviderSocket )
       )
    {
        SOCK_INFO   *NewSocketContext = NULL;

        //
        // Create socket context for new socket
        //
        NewSocketContext = CreateSockInfo(
                SocketContext->Provider,
                NextProviderSocket,
                SocketContext,
                FALSE,
                lpErrno
                );
        if  ( NULL == NewSocketContext )
        {
            goto cleanup;
        }

        //
        // Create a socket handle to pass to the app
        //
        NewSocket = NewSocketContext->LayeredSocket = gMainUpCallTable.lpWPUCreateSocketHandle(
                SocketContext->Provider->LayerProvider.dwCatalogEntryId,
                (DWORD_PTR) NewSocketContext,
                lpErrno
                );
        if ( INVALID_SOCKET == NewSocketContext->LayeredSocket )
        {
            int tempErr;

            ASSERT( SocketContext->Provider->NextProcTable.lpWSPCloseSocket );

            // Close the lower provider's socket
            SocketContext->Provider->NextProcTable.lpWSPCloseSocket(
                    NextProviderSocket,
                   &tempErr
                    );

            // Context is not in the list yet so we can just free it
            FreeSockInfo(NewSocketContext);
            goto cleanup;
        }

        // Need to insert the context
        InsertSocketInfo(SocketContext->Provider, NewSocketContext);
    }
    else if ( NextProviderSocket == SocketContext->ProviderSocket )
    {
        NewSocket = s;
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return NewSocket;
}

//
// Function: WSPListen
//
// Description:
//    This function sets the backlog value on a listening socket. All we need to
//    do is translate the socket handle to the correct provider.
//
int WSPAPI 
WSPListen(
    SOCKET s,        
    int    backlog,     
    LPINT  lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPListen: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPListen );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPListen(
            SocketContext->ProviderSocket, 
            backlog, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPRecv
//
// Description:
//    This function receives data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the receive call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPRecv(
    SOCKET          s,
    LPWSABUF        lpBuffers,
    DWORD           dwBufferCount,
    LPDWORD         lpNumberOfBytesRecvd,
    LPDWORD         lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID   lpThreadId,
    LPINT           lpErrno
    )
{
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    SOCK_INFO          *SocketContext = NULL;
    int                 ret = SOCKET_ERROR;


    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPRecv: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                SocketContext,
                LSP_OP_RECV,
                lpBuffers,
                dwBufferCount,
                lpOverlapped,
                lpCompletionRoutine,
                lpThreadId,
                lpErrno
                );
        if ( NULL == ProviderOverlapped )
            goto cleanup;

        __try
        {
            ProviderOverlapped->RecvArgs.dwNumberOfBytesRecvd = (lpNumberOfBytesRecvd ? *lpNumberOfBytesRecvd : 0);
            ProviderOverlapped->RecvArgs.dwFlags              = (lpFlags ? *lpFlags : 0);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
        ret = QueueOverlappedOperation(ProviderOverlapped, SocketContext);
        if ( NO_ERROR != ret )
        {
            *lpErrno = ret;
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ASSERT( SocketContext->Provider->NextProcTable.lpWSPRecv );

        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPRecv(
                SocketContext->ProviderSocket, 
                lpBuffers, 
                dwBufferCount,
                lpNumberOfBytesRecvd, 
                lpFlags, 
                lpOverlapped, 
                lpCompletionRoutine, 
                lpThreadId,
                lpErrno);
        SetBlockingProvider(NULL);
        if ( SOCKET_ERROR != ret )
        {
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPRecvDisconnect
//
// Description:
//    Receive data and disconnect. All we need to do is translate the socket
//    handle to the lower provider.
//
int WSPAPI 
WSPRecvDisconnect(
    SOCKET   s,
    LPWSABUF lpInboundDisconnectData,
    LPINT    lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPRecvDisconnect: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPRecvDisconnect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPRecvDisconnect(
            SocketContext->ProviderSocket,                           
            lpInboundDisconnectData, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPRecvFrom
//
// Description:
//    This function receives data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the receive call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPRecvFrom(
    SOCKET          s,
    LPWSABUF        lpBuffers,
    DWORD           dwBufferCount,
    LPDWORD         lpNumberOfBytesRecvd,
    LPDWORD         lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT           lpFromLen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID   lpThreadId,
    LPINT           lpErrno
    )
{
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;
    SOCK_INFO          *SocketContext = NULL;
    int                 ret = SOCKET_ERROR;

    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPRecvFrom: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    //
    // Check for overlapped I/O
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                SocketContext,
                LSP_OP_RECVFROM,
                lpBuffers,
                dwBufferCount,
                lpOverlapped,
                lpCompletionRoutine,
                lpThreadId,
                lpErrno
                );
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        __try 
        {
            ProviderOverlapped->RecvFromArgs.lpFrom        = lpFrom;
            ProviderOverlapped->RecvFromArgs.lpFromLen     = lpFromLen;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
        ret = QueueOverlappedOperation(ProviderOverlapped, SocketContext);
        if ( NO_ERROR != ret )
        {
            *lpErrno = ret;
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ASSERT( SocketContext->Provider->NextProcTable.lpWSPRecvFrom );

        // Make a blocking WSPRecvFrom call
        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPRecvFrom(
                SocketContext->ProviderSocket, 
                lpBuffers, 
                dwBufferCount,
                lpNumberOfBytesRecvd, 
                lpFlags, 
                lpFrom, 
                lpFromLen, 
                lpOverlapped, 
                lpCompletionRoutine, 
                lpThreadId, 
                lpErrno);
        SetBlockingProvider(NULL);
        if ( SOCKET_ERROR != ret )
        {
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: UnlockFdSets
//
// Description:
//      This is a utility function which iterates through the non-NULL, non-empty
//      fd_sets and its corresponding FD_MAP array. For each socket present in
//      the fd_set the corresponding socket context object is unlocked. This needs
//      to be performed before the application's fd_sets are modified otherwise
//      the mapping between fd_set and FD_MAP can get out of sync.
//
void
UnlockFdSets(
    fd_set  *readfds,
    FD_MAP  *readmap,
    fd_set  *writefds,
    FD_MAP  *writemap,
    fd_set  *exceptfds,
    FD_MAP  *exceptmap,
    LPINT    lpErrno
    )
{
    int     i;

    // Unlock socket contexts for the readfds sockets
    if ( NULL != readfds )
    {
        for(i=0; i < (int)readfds->fd_count ;i++)
        {
            if ( NULL != readmap[i].Context )
            {
                DerefSocketContext( readmap[i].Context, lpErrno );
                readmap[i].Context = NULL;
            }
        }
    }

    // Unlock socket contexts for the writefds sockets
    if ( NULL != writefds )
    {
        for(i=0; i < (int)writefds->fd_count ;i++)
        {
            if ( NULL != writemap[i].Context )
            {
                DerefSocketContext( writemap[i].Context, lpErrno );
                writemap[i].Context = NULL;
            }
        }
    }

    // Unlock socket contexts for the except sockets
    if ( NULL != exceptfds )
    {
        for(i=0; i < (int)exceptfds->fd_count ;i++)
        {
            if ( NULL != exceptmap[i].Context )
            {
                DerefSocketContext( exceptmap[i].Context, lpErrno );
                exceptmap[i].Context = NULL;
            }
        }
    }
}

//
// Function: WSPSelect
//
// Description:
//    This function tests a set of sockets for readability, writeability, and
//    exceptions. We must translate each handle in the fd_set structures to
//    their underlying provider handles before calling the next provider's
//    WSPSelect. The select API is really bad for LSPs in the sense multiple
//    provider's sockets can be passed into this provider's WSPSelect call.
//    If these unknown sockets (unknown since this LSP won't have a socket
//    context structure for it) are passed in the best we can do is pass
//    it down unmodified to the lower layer in the hopes that it knows what it
//    is. In the case where these unknown sockets belong to another LSP (which
//    isn't in our immediate chain) then the lower provider won't know about
//    that socket and will fail the call. Lastly we hold the context lock on
//    all sockets passed in until we're done.
//
int WSPAPI 
WSPSelect(
    int          nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT        lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    u_int      count,
               i;
    BOOL       unlocked = FALSE;
    int        HandleCount,
               ret SOCKET_ERROR;

    fd_set     ReadFds, 
               WriteFds, 
               ExceptFds;

    FD_MAP    *Read = NULL, 
              *Write = NULL, 
              *Except = NULL;

    if ( ( NULL == readfds ) && ( NULL == writefds ) && ( NULL == exceptfds ) )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    // Allocate storage to map upper level sockets to lower provider's sockets
    if ( NULL != readfds )
    {
        Read = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * readfds->fd_count, lpErrno );
        if ( NULL == Read )
            goto cleanup;
    }

    if ( NULL != writefds )
    {
        Write = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * writefds->fd_count, lpErrno );
        if ( NULL == Write )
            goto cleanup;
    }

    if ( NULL != exceptfds )
    {
        Except = (FD_MAP *) LspAlloc( sizeof( FD_MAP ) * exceptfds->fd_count, lpErrno );
        if ( NULL == Except )
            goto cleanup;
    }

    //
    // Translate all handles contained in the fd_set structures.
    //  For each fd_set go through and build another fd_set which contains
    //  their lower provider socket handles.
    //

    // Map the upper layer sockets to lower layer sockets in the write array
    if ( NULL != readfds )
    {
        FD_ZERO( &ReadFds );

        if ( readfds->fd_count > FD_SETSIZE )
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }

        for (i = 0; i < readfds->fd_count; i++)
        {
            Read[i].Context = FindAndRefSocketContext(
                    (Read[i].ClientSocket = readfds->fd_array[i]),
                    lpErrno
                    );
            if ( NULL == Read[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Read[i].ProvSocket = readfds->fd_array[i];
                FD_SET(Read[i].ProvSocket, &ReadFds);
            }
            else
            {
                Read[i].ProvSocket = Read[i].Context->ProviderSocket;
                FD_SET(Read[i].ProvSocket, &ReadFds);

                // Save the first valid socket context structure
                if ( NULL == SocketContext )
                    SocketContext = Read[i].Context;
            }
        }
    }

    // Map the upper layer sockets to lower layer sockets in the write array
    if ( NULL != writefds )
    {
        FD_ZERO( &WriteFds );

        if ( writefds->fd_count > FD_SETSIZE )
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }
        for (i = 0; i < writefds->fd_count; i++)
        {
            Write[i].Context = FindAndRefSocketContext(
                    (Write[i].ClientSocket = writefds->fd_array[i]), 
                    lpErrno
                    );
            if ( NULL == Write[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Write[i].ProvSocket = writefds->fd_array[i];
                FD_SET(Write[i].ProvSocket, &WriteFds);
            }
            else
            {
                Write[i].ProvSocket = Write[i].Context->ProviderSocket;
                FD_SET(Write[i].ProvSocket, &WriteFds);

                // Save the first valid socket context structure
                if ( NULL == SocketContext )
                    SocketContext = Write[i].Context;
            }
        }
    }

    // Map the upper layer sockets to lower layer sockets in the except array
    if ( NULL != exceptfds )
    {
        FD_ZERO( &ExceptFds );

        if (exceptfds->fd_count > FD_SETSIZE)
        {
            *lpErrno = WSAENOBUFS;
            goto cleanup;
        }
        for (i = 0; i < exceptfds->fd_count; i++)
        {
            Except[i].Context = FindAndRefSocketContext(
                    (Except[i].ClientSocket = exceptfds->fd_array[i]), 
                    lpErrno
                    );
            if ( NULL == Except[i].Context )
            {
                // This socket isn't ours -- just pass down in hopes the lower provider
                // knows about it
                Except[i].ProvSocket = exceptfds->fd_array[i];
                FD_SET(Except[i].ProvSocket, &ExceptFds);
            }
            else
            {
                Except[i].ProvSocket = Except[i].Context->ProviderSocket;
                FD_SET(Except[i].ProvSocket, &ExceptFds);

                // Save the first valid socket context structure
                if ( NULL == SocketContext )
                    SocketContext = Except[i].Context;
            }
        }
    }

    //
    // Now call the lower provider's WSPSelect with the fd_set structures we built
    //  containing the lower provider's socket handles.
    //
    if ( NULL == SocketContext )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPSelect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPSelect(
            nfds, 
            (readfds ? &ReadFds : NULL), 
            (writefds ? &WriteFds : NULL), 
            (exceptfds ? &ExceptFds : NULL), 
            timeout, 
            lpErrno
            );
    SetBlockingProvider(NULL);

    // Need to unlock the contexts before the original fd_sets are modified
    UnlockFdSets( readfds, Read, writefds, Write, exceptfds, Except, lpErrno );
    unlocked = TRUE;

    if ( SOCKET_ERROR != ret )
    {
        // Once we complete we now have to go through the fd_sets we passed and
        //  map them BACK to the application socket handles. Fun!
        //
        HandleCount = ret;

        if ( NULL != readfds )
        {
            count = readfds->fd_count;
            FD_ZERO(readfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Read[i].ProvSocket, &ReadFds) )
                {
                    FD_SET(Read[i].ClientSocket, readfds);
                    HandleCount--;
                }
            }
        }

        if ( NULL != writefds )
        {
            count = writefds->fd_count;
            FD_ZERO(writefds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Write[i].ProvSocket, &WriteFds) )
                {
                    FD_SET(Write[i].ClientSocket, writefds);
                    HandleCount--;
                }
            }
        }

        if ( NULL != exceptfds )
        {
            count = exceptfds->fd_count;
            FD_ZERO(exceptfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if ( gMainUpCallTable.lpWPUFDIsSet(Except[i].ProvSocket, &ExceptFds) )
                {
                    FD_SET(Except[i].ClientSocket, exceptfds);
                    HandleCount--;
                }
            }
        }
    }

cleanup:

    // In case of error, ensure the socket contexts get unlocked
    if ( FALSE == unlocked )
        UnlockFdSets( readfds, Read, writefds, Write, exceptfds, Except, lpErrno );

    // Unlock socket context here in case an error occurs
    if ( NULL != Read )
        LspFree( Read );

    if ( NULL != Write )
        LspFree( Write );

    if ( NULL != Except )
        LspFree( Except );

    return ret;
}

//
// Function: WSPSend
//
// Description:
//    This function sends data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the send call. If called with overlap, post the operation
//    to our IOCP or completion routine.
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
    INT                 ret = SOCKET_ERROR;
    SOCK_INFO          *SocketContext = NULL;
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;

    *lpErrno = NO_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPSend: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    //
    // Check for overlapped I/O
    // 
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                SocketContext,
                LSP_OP_SEND,
                lpBuffers,
                dwBufferCount,
                lpOverlapped,
                lpCompletionRoutine,
                lpThreadId,
                lpErrno
                );
        if ( NULL == ProviderOverlapped )
        {
            goto cleanup;
        }

        __try 
        {
            ProviderOverlapped->SendArgs.dwFlags       = dwFlags;
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
        ret = QueueOverlappedOperation(ProviderOverlapped, SocketContext);
        if ( NO_ERROR != ret )
        {
            *lpErrno = ret;
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ASSERT( SocketContext->Provider->NextProcTable.lpWSPSend );

        // Make a blocking send call
        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPSend(
                SocketContext->ProviderSocket, 
                lpBuffers, 
                dwBufferCount,
                lpNumberOfBytesSent, 
                dwFlags, 
                lpOverlapped, 
                lpCompletionRoutine, 
                lpThreadId, 
                lpErrno
                );
        SetBlockingProvider(NULL);
        if ( SOCKET_ERROR != ret )
        {
            SocketContext->BytesSent += *lpNumberOfBytesSent;
        }
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPSendDisconnect
//
// Description:
//    Send data and disconnect. All we need to do is translate the socket
//    handle to the lower provider.
//
int WSPAPI 
WSPSendDisconnect(
    SOCKET   s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT    lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPSendDisconnect: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPSendDisconnect );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPSendDisconnect(
            SocketContext->ProviderSocket,
            lpOutboundDisconnectData, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPSendTo
//
// Description:
//    This function sends data on a given socket and also allows for asynchronous
//    (overlapped) operation. First translate the socket handle to the lower provider
//    handle and then make the send call. If called with overlap, post the operation
//    to our IOCP or completion routine.
//
int WSPAPI 
WSPSendTo(
    SOCKET          s,
    LPWSABUF        lpBuffers,
    DWORD           dwBufferCount,
    LPDWORD         lpNumberOfBytesSent,
    DWORD           dwFlags,
    const struct sockaddr FAR * lpTo,
    int             iToLen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID   lpThreadId,
    LPINT           lpErrno
    )
{
    int                 ret = SOCKET_ERROR;
    SOCK_INFO          *SocketContext = NULL;
    LPWSAOVERLAPPEDPLUS ProviderOverlapped = NULL;

    //
    // Find our provider socket corresponding to this one
    //
    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPSendTo: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    //
    // Check for overlapped
    //
    if ( NULL != lpOverlapped )
    {
        ProviderOverlapped = PrepareOverlappedOperation(
                SocketContext,
                LSP_OP_SENDTO,
                lpBuffers,
                dwBufferCount,
                lpOverlapped,
                lpCompletionRoutine,
                lpThreadId,
                lpErrno
                );
        if ( NULL == ProviderOverlapped )
            goto cleanup;

        __try 
        {
            ProviderOverlapped->SendToArgs.dwFlags             = dwFlags;
            ProviderOverlapped->SendToArgs.iToLen              = iToLen;
            ProviderOverlapped->SendToArgs.dwNumberOfBytesSent = (lpNumberOfBytesSent ? *lpNumberOfBytesSent : 0);
            if ( iToLen <= sizeof( ProviderOverlapped->SendToArgs.To ) )
                CopyMemory(&ProviderOverlapped->SendToArgs.To, lpTo, iToLen);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            // Return to original state and indicate error
            *lpErrno = WSAEFAULT;
            goto cleanup;
        }

        //
        // Make the overlapped call which will always fail (either with WSA_IO_PENDING
        // or actual error code if something is wrong).
        //
        ret = QueueOverlappedOperation(ProviderOverlapped, SocketContext);
        if ( NO_ERROR != ret )
        {
            *lpErrno = ret;
            ret = SOCKET_ERROR;
        }
    }
    else
    {
        ASSERT( SocketContext->Provider->NextProcTable.lpWSPSendTo );

        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPSendTo(
                SocketContext->ProviderSocket, 
                lpBuffers, 
                dwBufferCount,
                lpNumberOfBytesSent, 
                dwFlags, 
                lpTo, 
                iToLen, 
                lpOverlapped, 
                lpCompletionRoutine, 
                lpThreadId, 
                lpErrno
                );
        SetBlockingProvider(NULL);
        if ( SOCKET_ERROR != ret )
        {
            SocketContext->BytesSent += *lpNumberOfBytesSent;
        }
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPSetSockOpt
//
// Description:
//    Set a socket option. For most all options we just have to translate the
//    socket option and call the lower provider. The only special case is for
//    SO_UPDATE_ACCEPT_CONTEXT in which case a socket handle is passed as the
//    argument which we need to translate before calling the lower provider.
//
int WSPAPI WSPSetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    const char FAR * optval,   
    int        optlen,
    LPINT      lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL,
              *AcceptContext = NULL;
    INT        ret = SOCKET_ERROR;

    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPSetSockOpt: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPSetSockOpt );

    if ( SO_UPDATE_ACCEPT_CONTEXT == optname )
    {
        // We need to intercept this (and any other options) that pass
        //  a socket handle as an argument so we can replace it with the
        //  correct underlying provider's socket handle.
        //
        AcceptContext = FindAndRefSocketContext( *((SOCKET *)optval), lpErrno);
        if ( NULL == AcceptContext )
        {
            dbgprint( "WSPSetSockOpt: SO_UPDATE_ACCEPT_CONTEXT: FindAndRefSocketContext failed!" );
            goto cleanup;
        }

        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPSetSockOpt(
                SocketContext->ProviderSocket, 
                level,
                optname, 
                (char *)&AcceptContext->ProviderSocket, 
                optlen, 
                lpErrno
                );
        SetBlockingProvider(NULL);
    }
    else
    {
        SetBlockingProvider(SocketContext->Provider);
        ret = SocketContext->Provider->NextProcTable.lpWSPSetSockOpt(
                SocketContext->ProviderSocket, 
                level,                 
                optname, 
                optval, 
                optlen, 
                lpErrno
                );
        SetBlockingProvider(NULL);
    }

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    if ( NULL != AcceptContext )
        DerefSocketContext( AcceptContext, lpErrno );

    return ret;
}

//
// Function: WSPShutdown
//
// Description:
//    This function performs a shutdown on the socket. All we need to do is 
//    translate the socket handle to the lower provider.
//
int WSPAPI 
WSPShutdown (
    SOCKET s,
    int    how,
    LPINT  lpErrno
    )
{
    SOCK_INFO *SocketContext = NULL;
    INT        ret = SOCKET_ERROR;

    SocketContext = FindAndRefSocketContext(s, lpErrno);
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPShutdown: FindAndRefSocketContext failed!" );
        goto cleanup;
    }

    ASSERT( SocketContext->Provider->NextProcTable.lpWSPShutdown );

    SetBlockingProvider(SocketContext->Provider);
    ret = SocketContext->Provider->NextProcTable.lpWSPShutdown(
            SocketContext->ProviderSocket, 
            how, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    if ( NULL != SocketContext )
        DerefSocketContext( SocketContext, lpErrno );

    return ret;
}

//
// Function: WSPStringToAddress
//
// Description:
//    Convert a string to an address (SOCKADDR structure).  We need to translate
//    the socket handle as well as possibly substitute the lpProtocolInfo structure
//    passed to the next provider. 
//
int WSPAPI 
WSPStringToAddress(
    LPWSTR              AddressString,
    INT                 AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR          lpAddress,
    LPINT               lpAddressLength,
    LPINT               lpErrno
    )
{
    WSAPROTOCOL_INFOW   *pInfo = NULL;
    PROVIDER            *Provider = NULL;
    INT                  ret = SOCKET_ERROR,
                         i;

    for(i=0; i < gLayerCount ;i++)
    {
        if ( ( gBaseInfo[i].NextProvider.iAddressFamily == lpProtocolInfo->iAddressFamily ) &&
             ( gBaseInfo[i].NextProvider.iSocketType == lpProtocolInfo->iSocketType ) && 
             ( gBaseInfo[i].NextProvider.iProtocol   == lpProtocolInfo->iProtocol )
           )
        {
            if ( NULL != lpProtocolInfo )
            {
                // In case of multiple providers check the provider flags 
                if ( ( gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES ) != 
                     ( lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES ) 
                   )
                {
                    continue;
                }
            }
            Provider = &gBaseInfo[i];
            pInfo = &gBaseInfo[i].NextProvider;
            break;
        }
    }

    if ( NULL == Provider )
    {
        *lpErrno = WSAEINVAL;
        goto cleanup;
    }

    //
    // If we're not immediately above the base then pass the lpProtocolInfo passed
    // into us.
    //
    if ( BASE_PROTOCOL != pInfo->ProtocolChain.ChainLen )
    {
        pInfo = lpProtocolInfo;
    }

    if ( 0 == Provider->StartupCount )
    {
        if ( SOCKET_ERROR == InitializeProvider( Provider, MAKEWORD(2,2), lpProtocolInfo,
                gMainUpCallTable, lpErrno ) )
        {
            dbgprint("WSPStringToAddress: InitializeProvider failed: %d", *lpErrno );
            goto cleanup;
        }
    }

    ASSERT( Provider->NextProcTable.lpWSPStringToAddress );

    SetBlockingProvider(Provider);
    ret = Provider->NextProcTable.lpWSPStringToAddress(
            AddressString, 
            AddressFamily,
            pInfo, 
            lpAddress, 
            lpAddressLength, 
            lpErrno
            );
    SetBlockingProvider(NULL);

cleanup:

    return ret;
}

//
// Function: WSPSocket
//
// Description:
//    This function creates a socket. There are two sockets created. The first
//    socket is created by calling the lower providers WSPSocket. This is the
//    handle that we use internally within our LSP. We then create a second
//    socket with WPUCreateSocketHandle which will be returned to the calling
//    application. We will also create a socket context structure which will
//    maintain information on each socket. This context is associated with the
//    socket handle passed to the application.
//
SOCKET WSPAPI 
WSPSocket(
    int                 af,
    int                 type,
    int                 protocol,
    __in LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP               g,
    DWORD               dwFlags,
    LPINT               lpErrno
    )
{
    SOCKET              NextProviderSocket = INVALID_SOCKET;
    SOCKET              NewSocket = INVALID_SOCKET;
    SOCK_INFO          *SocketContext = NULL;
    WSAPROTOCOL_INFOW  *pInfo = NULL, 
                        InfoCopy;
    PROVIDER           *Provider = NULL;
    BOOL                bAddressFamilyOkay = FALSE,
                        bSockTypeOkay = FALSE,
                        bProtocolOkay = FALSE,
                        bAFTypeMatch = FALSE;
    INT                 iAddressFamily,
                        iSockType, 
                        iProtocol, 
                        i;

    *lpErrno = NO_ERROR;

    //
    // If a WSAPROTOCOL_INFO structure was passed in, use those socket/protocol
    //  values. Then find the underlying provider's WSAPROTOCOL_INFO structure.
    //
    iAddressFamily = (lpProtocolInfo ? lpProtocolInfo->iAddressFamily : af);
    iProtocol      = (lpProtocolInfo ? lpProtocolInfo->iProtocol   : protocol);
    iSockType      = (lpProtocolInfo ? lpProtocolInfo->iSocketType : type);

    #ifdef DEBUG
    if (lpProtocolInfo)
        dbgprint("WSPSocket: Provider: '%S'", lpProtocolInfo->szProtocol);
    #endif

    for(i=0; i < gLayerCount ;i++)
    {
        if ( ( iAddressFamily == AF_UNSPEC) ||
             ( iAddressFamily == gBaseInfo[i].NextProvider.iAddressFamily )
           )
        {
            bAddressFamilyOkay = TRUE;
        }
        if ( iSockType == gBaseInfo[i].NextProvider.iSocketType )
        {
            bSockTypeOkay = TRUE;
        }
        if ( ( iProtocol == 0) || 
             ( iProtocol == gBaseInfo[i].NextProvider.iProtocol) ||
             ( iProtocol == IPPROTO_RAW) || 
              (iSockType == SOCK_RAW )
           )
        {
            bProtocolOkay = TRUE;
        }
    }
    if ( FALSE == bAddressFamilyOkay )
    {
        *lpErrno = WSAEAFNOSUPPORT;
        goto cleanup;
    }
    if ( FALSE == bSockTypeOkay )
    {
        *lpErrno = WSAESOCKTNOSUPPORT;
        goto cleanup;
    }
    if ( FALSE == bProtocolOkay )
    {
        *lpErrno = WSAEPROTONOSUPPORT;
        goto cleanup;
    }

    //
    // If AF_UNSPEC was passed in we need to go by the socket type and protocol
    //  if possible.
    //
    if ( ( AF_UNSPEC == iAddressFamily ) && ( 0 == iProtocol ) )
    {
        for(i=0; i < gLayerCount ;i++)
        {
            if ( gBaseInfo[i].NextProvider.iSocketType == iSockType )
            {
                if ( NULL != lpProtocolInfo )
                {
                    // In case of multiple providers check the provider flags 
                    if ( (gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES) != 
                         (lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES) )
                    {
                        continue;
                    }
                }
                Provider = &gBaseInfo[i];
                pInfo = &gBaseInfo[i].NextProvider;
                //if ( NULL != lpProtocolInfo )
                //    pInfo->dwProviderReserved = lpProtocolInfo->dwProviderReserved;
                break;
            }
        }
    }
    else if ( ( AF_UNSPEC == iAddressFamily ) && ( 0 != iProtocol ) )
    {
        for(i=0; i < gLayerCount ;i++)
        {
            if ( ( gBaseInfo[i].NextProvider.iProtocol == iProtocol ) &&
                 ( gBaseInfo[i].NextProvider.iSocketType == iSockType ) 
               )
            {
                if ( NULL != lpProtocolInfo )
                {
                    // In case of multiple providers check the provider flags 
                    if ( (gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES) != 
                         (lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES) )
                    {
                        continue;
                    }
                }
                Provider = &gBaseInfo[i];
                pInfo = &gBaseInfo[i].NextProvider;
                //if ( NULL != lpProtocolInfo )
                //    pInfo->dwProviderReserved = lpProtocolInfo->dwProviderReserved;
                break;
            }
        }
        if ( NULL == pInfo )
        {
            *lpErrno = WSAEPROTOTYPE;
            goto cleanup;
        }
    }
    else if ( ( 0 != iProtocol ) && 
              ( IPPROTO_RAW != iProtocol ) && 
              ( SOCK_RAW != iSockType ) 
            )
    {
        for(i=0; i < gLayerCount ;i++)
        {
            if ((gBaseInfo[i].NextProvider.iAddressFamily == iAddressFamily) &&
                (gBaseInfo[i].NextProvider.iSocketType == iSockType))
            {
                bAFTypeMatch = TRUE;

                if (gBaseInfo[i].NextProvider.iProtocol == iProtocol)
                {
                    if ( NULL != lpProtocolInfo )
                    {
                        // In case of multiple providers check the provider flags 
                        if ( (gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES) != 
                             (lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES) )
                        {
                            continue;
                        }
                    }
                    Provider = &gBaseInfo[i];
                    pInfo = &gBaseInfo[i].NextProvider;
                    //if ( NULL != lpProtocolInfo )
                    //    pInfo->dwProviderReserved = lpProtocolInfo->dwProviderReserved;
                    break;
                }
            }
        }
        
        if ( NULL == pInfo && TRUE == bAFTypeMatch )
        {
            *lpErrno = WSAEPROTOTYPE;
            goto cleanup;
        }
    }
    else
    {
        for(i=0; i < gLayerCount ;i++)
        {
            if ( ( gBaseInfo[i].NextProvider.iAddressFamily == iAddressFamily ) &&
                 ( gBaseInfo[i].NextProvider.iSocketType == iSockType )
               )
            {
                if ( NULL != lpProtocolInfo )
                {
                    // In case of multiple providers check the provider flags 
                    if ( (gBaseInfo[i].NextProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES) != 
                         (lpProtocolInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES) )
                    {
                        continue;
                    }
                }
                Provider = &gBaseInfo[i];
                pInfo = &gBaseInfo[i].NextProvider;
                //if ( NULL != lpProtocolInfo )
                //    pInfo->dwProviderReserved = lpProtocolInfo->dwProviderReserved;
                break;
            }
        }
    }
    if ( NULL == Provider )
    {
        *lpErrno = WSAEAFNOSUPPORT;
        return INVALID_SOCKET;
    }

    if ( BASE_PROTOCOL != pInfo->ProtocolChain.ChainLen )
    {
        pInfo = lpProtocolInfo;
    }

    memcpy(&InfoCopy, pInfo, sizeof(InfoCopy));

    if ( NULL != lpProtocolInfo )
    {
        InfoCopy.dwProviderReserved = lpProtocolInfo->dwProviderReserved;
        if ( InfoCopy.dwProviderReserved )
            dbgprint("WSPSocket: dwProviderReserved = %d", InfoCopy.dwProviderReserved );
    }

    if ( 0 == Provider->StartupCount ) 
    {
        if ( SOCKET_ERROR == InitializeProvider( Provider, MAKEWORD(2,2), lpProtocolInfo, 
                gMainUpCallTable, lpErrno ) )
        {
            dbgprint("WSPSocket: InitializeProvider failed: %d", *lpErrno );
            goto cleanup;
        }
    }

    //
    // Create the underlying provider's socket.
    //

    dbgprint("Calling the lower provider WSPSocket: '%S'", Provider->NextProvider.szProtocol);

    ASSERT( Provider->NextProcTable.lpWSPSocket );

    SetBlockingProvider(Provider);
    NextProviderSocket = Provider->NextProcTable.lpWSPSocket(
            af, 
            type, 
            protocol, 
           &InfoCopy,
            g, 
            dwFlags | WSA_FLAG_OVERLAPPED, // Always Or in the overlapped flag
            lpErrno
            );
    SetBlockingProvider(NULL);

    if ( INVALID_SOCKET == NextProviderSocket )
    {
        dbgprint("WSPSocket: NextProcTable.WSPSocket() failed: %d", *lpErrno);
        goto cleanup;
    }

    //
    // Create the context information to be associated with this socket
    //
    SocketContext = CreateSockInfo(
            Provider,
            NextProviderSocket,
            NULL,
            TRUE,
            lpErrno
            );
    if ( NULL == SocketContext )
    {
        dbgprint( "WSPSocket: CreateSockInfo failed: %d", *lpErrno );
        goto cleanup;
    }

    //
    // Create a socket handle to pass back to app
    //  
    NewSocket = gMainUpCallTable.lpWPUCreateSocketHandle(
            Provider->LayerProvider.dwCatalogEntryId,
            (DWORD_PTR) SocketContext, 
            lpErrno
            );
    if ( INVALID_SOCKET == NewSocket )
    {
        int tempErr;

        dbgprint("WSPSocket: WPUCreateSocketHandle() failed: %d", *lpErrno);

        Provider->NextProcTable.lpWSPCloseSocket(
                NextProviderSocket, 
               &tempErr
                );

        goto cleanup;
    }

    dbgprint("Lower provider socket = 0x%x  LSP Socket = 0x%x\n", NextProviderSocket, NewSocket);

    SocketContext->LayeredSocket = NewSocket;

    //pInfo->dwProviderReserved = 0;

    return NewSocket;

cleanup:

    if ( ( NULL != Provider ) && ( NULL != SocketContext ) )
        RemoveSocketInfo(Provider, SocketContext);

    if ( NULL != SocketContext )
        FreeSockInfo(SocketContext);

    return INVALID_SOCKET;
}

//
// Function: WSPStartup
//
// Description:
//    This function intializes our LSP. We maintain a ref count to keep track
//    of how many times this function has been called. On the first call we'll
//    look at the Winsock catalog to find our catalog ID and find which entries
//    we are layered over. We'll create a number of structures to keep this 
//    information handy.
//
//    NOTE: There are two basic methods of finding an LSPs position in the chain.
//          First, it may look at the protocol chain of the lpProtocolInfo passed
//          in. If it does this an LSP SHOULD find its position by matching ANY
//          of the provider IDs beloging to the LSP. This includes the ID of the
//          dummy entry as well as the IDs of any of the layered providers belonging
//          to the LSP!
//
//          The second option is to enumerate the Winsock catalog and find find
//          all the entries belonging to this LSP. Then from the layered provider 
//          entries, take position 1 in the protocol chain to find out who is next
//          in your layer and load them.
//
//          This LSP takes the second approach -- mainly because if another LSP
//          layers on top of us and dorks up their protocol chain, if this LSP is
//          called it can reliably load the lower provider (unless the bad LSP
//          modified our LSPs entries and goofed up our chains as well).
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
    INT                 ret,
                        Error = WSAEPROVIDERFAILEDINIT;

    EnterCriticalSection( &gCriticalSection );

    //
    // Load Next Provider in chain if this is the first time called
    //
    if ( 0 == gEntryCount )
    {
        ret = LspCreateHeap( &Error );
        if ( SOCKET_ERROR == ret )
        {
            dbgprint("WSPStartup: LspCreateHeap failed: %d", Error );
            goto cleanup;
        }

        memcpy( &gMainUpCallTable, &UpCallTable, sizeof( gMainUpCallTable ) );

        // Create an event which is signaled when a socket context is added
        gAddContextEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( NULL == gAddContextEvent )
        {
            dbgprint("WSPStartup: CreateEvent failed: %d", GetLastError());
            goto cleanup;
        }

        ret = FindLspEntries( &gBaseInfo, &gLayerCount, &Error );
        if ( FALSE == ret )
        {
            dbgprint("WSPStartup: FindLspEntries failed: %d", Error );
            goto cleanup;
        }

        //
        // Initialize the overlapped manager. This creates the completion port used
        // by the LSP which needs to occur here since other parts of the LSP use the
        // gIocp variable to determine whether we're on Win9x or NT.
        //
        Error = InitOverlappedManager();
    }

    loadProvider = FindMatchingLspEntryForProtocolInfo(
            lpProtocolInfo,
            gBaseInfo,
            gLayerCount,
            TRUE
            );
    if ( NULL == loadProvider )
    {
        dbgprint("WSPStartup: FindMatchingLspEntryForProtocolInfo failed!");
        ASSERT( 0 );
        goto cleanup;
    }

    if ( 0 == loadProvider->StartupCount )
    {
        if ( SOCKET_ERROR == InitializeProvider( loadProvider, wVersion, lpProtocolInfo, 
                UpCallTable, &Error ) )
        {
            dbgprint("WSPStartup: InitializeProvider failed: %d", Error );
            goto cleanup;
        }
    }

    gEntryCount++;

    //
    // Copy the LSP's proc table to the caller's data structures
    //
    memcpy( lpWSPData, &loadProvider->WinsockVersion, sizeof( WSPDATA ) );
    memcpy( lpProcTable, &gProcTable, sizeof( WSPPROC_TABLE ) );

    Error = NO_ERROR;

cleanup:

    if ( NO_ERROR != Error )
    {
        dbgprint( "WSPStartup failed!" );

        // Had errors during initialization and gEntryCount is still zero so
        // cleanup the internal structures
        FreeSocketsAndMemory( FALSE, &Error );
    }

    LeaveCriticalSection( &gCriticalSection );

    return Error;
}

//
// Function: CopyOffset
//
// Description:
//    Any offset information passed by the application in its OVERLAPPED structure
//    needs to be copied down to the OVERLAPPED structure the LSP passes to the
//    lower layer. This function copies the offset fields.
//
void 
CopyOffset(
    WSAOVERLAPPED  *ProviderOverlapped, 
    WSAOVERLAPPED  *UserOverlapped
    )
{
    ProviderOverlapped->Offset     = UserOverlapped->Offset;
    ProviderOverlapped->OffsetHigh = UserOverlapped->OffsetHigh;
}

//
// Function: CopyWSABuf
//
// Description:
//    Overlapped send/recv functions pass an array of WSABUF structures to specify
//    the send/recv buffers and their lengths. The Winsock spec says that providers
//    must capture all the WSABUF structures and cannot rely on them being persistent.
//    If we're on NT then we don't have to copy as we immediately call the lower
//    provider's function (and the lower provider captures the WSABUF array). However
//    if the LSP is modified to look at the buffers after the operaiton is queued, 
//    then this routine must ALWAYS copy the WSABUF array.  For Win9x since the 
//    overlapped operation doesn't immediately execute we have to copy the array.
//
WSABUF *
CopyWSABuf(
    WSABUF *BufferArray, 
    DWORD   BufferCount, 
    int    *lpErrno
    )
{
    WSABUF      *buffercopy = NULL;
    DWORD        i;

    if ( NULL == gIocp )
    {
        //
        // We're on Win9x -- we need to save off the WSABUF structures
        // because on Win9x, the overlapped operation does not execute
        // immediately and the Winsock spec says apps are free to use
        // stack based WSABUF arrays.
        //
        
        buffercopy = (WSABUF *) LspAlloc(
                sizeof(WSABUF) * BufferCount,
                lpErrno
                );
        if ( NULL == buffercopy )
        {
            dbgprint( "CopyWSABuf: HeapAlloc failed: %d", GetLastError() );
            return NULL;
        }

        for(i=0; i < BufferCount ;i++)
        {
            buffercopy[i].buf = BufferArray[i].buf;
            buffercopy[i].len = BufferArray[i].len;
        }

        return buffercopy;
    }
    else
    {
        // With completion ports, we post the overlapped operation
        // immediately to the lower provider which should capture
        // the WSABUF array members itself. If your LSP needs to
        // look at the buffers after the operation is initiated,
        // you'd better always copy the WSABUF array.

        return BufferArray;
    }
}

//
// Function: FreeWSABuf
//
// Description:
//    Read the description for CopyWSABuf first! This routine frees the allocated
//    array of WSABUF structures. Normally, the array is copied only on Win9x 
//    systems. If your LSP needs to look at the buffers after the overlapped operation
//    is issued, you must always copy the buffers (therefore you must always delete
//    them when done).
//
void 
FreeWSABuf(
    WSABUF *BufferArray
    )
{
    if ( ( NULL == gIocp ) && ( NULL != BufferArray ) )
    {
        // If we're on Win9x, the WSABUF array was copied so free it up now

        LspFree( BufferArray );
    }
}

//
// Function: FreeSocketsAndMemory
//
// Description:
//    Go through each provider and close all open sockets. Then call each
//    underlying provider's WSPCleanup and free the reference to its DLL.
//
void 
FreeSocketsAndMemory(
    BOOL processDetach,
    int *lpErrno
    )
{
    int     ret,
            i;

    if ( NULL != gBaseInfo )
    {
        // Walk through each PROVIDER entry in the array
        for(i=0; i < gLayerCount ;i++)
        {
            if ( NULL != gBaseInfo[i].Module )
            {
                //
                // Close all sockets created from this provider
                //
                CloseAndFreeSocketInfo( &gBaseInfo[i], processDetach );

                //
                // Call the WSPCleanup of the provider's were layered over.
                //

                if ( ( !processDetach ) || 
                     ( gBaseInfo[ i ].NextProvider.ProtocolChain.ChainLen == BASE_PROTOCOL ) )
                {
                    while( 0 != gBaseInfo[ i ].StartupCount )
                    {
                        gBaseInfo[ i ].StartupCount--;

                        if ( gBaseInfo[i].NextProcTable.lpWSPCleanup != NULL )
                            ret = gBaseInfo[i].NextProcTable.lpWSPCleanup( lpErrno );
                    }
                }

                DeleteCriticalSection( &gBaseInfo[i].ProviderCritSec );

                if ( NULL != gBaseInfo[i].Module )
                    FreeLibrary( gBaseInfo[i].Module );

                gBaseInfo[i].Module = NULL;
            }
        }

        LspFree( gBaseInfo );
        gBaseInfo = NULL;
    }

    if ( NULL != gAddContextEvent )
    {
        CloseHandle( gAddContextEvent );
        gAddContextEvent = NULL;
    }

    LspDestroyHeap();
}
