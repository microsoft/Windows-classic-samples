// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: extension.cpp
//
// Description:
//
//    This sample illustrates an IFS based LSP which implements basic proxying
//    capabilities.
//
//    This file contains all of the Winsock extension functions that the LSP needs
//    to intercept in order to proxy connections. In terms of Winsock extension
//    functions this means only the ConnectEx function needs to be intercepted.
//

#include "lspdef.h"

//
// Function: LoadExtensionFunction
//
// Description:
//    This function dynamically loads the given extension function from the
//    underlying provider. Each extension function checks to see if the 
//    corresponding extension function for the lower provider is loaded
//    before calling. If not, it will load it as needed. This is necessary
//    if the app loads the extension function for say TCP and then calls
//    that extension function on a UDP socket. Normally this isn't the case
//    but we're being defensive here.
//
BOOL
LoadExtensionFunction(
        FARPROC   **func,
        GUID        ExtensionGuid,
        LPWSPIOCTL  fnIoctl,
        SOCKET      s
        )
{
    DWORD   dwBytes;
    int     rc, 
            error;

    // Use the lower provider's WSPIoctl to load the extension function
    rc = fnIoctl(
            s,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
           &ExtensionGuid,
            sizeof(GUID),
            func,
            sizeof(FARPROC),
           &dwBytes,
            NULL,
            NULL,
            NULL,
           &error
            );
    if ( SOCKET_ERROR == rc )
    {
        dbgprint("LoadExtensionFunction: WSAIoctl (SIO_GET_EXTENSION_FUNCTION) failed: %d",
            error);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//
// Function: ExtConnectEx
//
// Description:
//    This is our provider's ConnectEx function. When an app calls WSAIoctl
//    to request the function pointer to ConnectEx, we intercept the call
//    and return a pointer to our extension function instead. This ConnectEx
//    implementation needs to perform the same proxying check that WSPConnect
//    does.
//
BOOL PASCAL FAR 
ExtConnectEx(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    )
{
    SOCKET_CONTEXT *sockContext = NULL;
    SOCKADDR       *proxyAddr = NULL;
    int             Errno = NO_ERROR,
                    proxyLen = 0,
                    rc = FALSE;

    sockContext = FindSocketContext( s );
    if ( NULL == sockContext )
    {
        dbgprint("ExtConnectEx: FindSocketContext failed!");
        Errno = WSAENOTSOCK;
        goto cleanup;
    }

    // Make sure we already have the extension function
    if ( NULL == sockContext->Provider->NextProcTableExt.lpfnConnectEx )
    {
        GUID    guidConnectEx = WSAID_CONNECTEX;

        rc = LoadExtensionFunction(
                (FARPROC **)&sockContext->Provider->NextProcTableExt.lpfnConnectEx,
                guidConnectEx,
                sockContext->Provider->NextProcTable.lpWSPIoctl,
                s
                );
        if ( FALSE == rc )
        {
            dbgprint("Next proc table ConnectEx == NULL!");
            Errno = WSAEFAULT;
            goto cleanup;
        }
    }

    // See if the connect needs to be proxied
    FindDestinationAddress( sockContext, name, namelen, &proxyAddr, &proxyLen );

    rc = sockContext->Provider->NextProcTableExt.lpfnConnectEx(
            s,
            proxyAddr,
            proxyLen,
            lpSendBuffer,
            dwSendDataLength,
            lpdwBytesSent,
            lpOverlapped
            );

cleanup:

    if ( NO_ERROR != Errno )
        WSASetLastError( Errno );

    return rc;
}
