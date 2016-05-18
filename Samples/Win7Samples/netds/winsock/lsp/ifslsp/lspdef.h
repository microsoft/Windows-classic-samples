// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
// 
// Module Name: provider.h
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains all datatypes and function prototypes used
//    throughout this project.
//
#ifndef _PROVIDER_H_
#define _PROVIDER_H_ 

#ifndef _PSDK_BLD
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <ws2spi.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#ifndef _PSDK_BLD
#include <lspcommon.h>
#else
#include "..\common\lspcommon.h"
#endif

//
// This is the socket context data that we associate with each socket
//  that is passed back to the user app. That way when another API
//  is called we can query for this context information and find out
//  the socket handle to the lower provider.
//
typedef struct _SOCKET_CONTEXT
{
    SOCKET              Socket;     // Lower provider socket handle
    PROVIDER           *Provider;   // Pointer to the provider from which socket was created

    SOCKADDR_STORAGE    ProxiedAddress;
    SOCKADDR_STORAGE    OriginalAddress;

    int                 AddressLength;

    BOOL                Proxied;

    LIST_ENTRY          Link;

} SOCKET_CONTEXT;

////////////////////////////////////////////////////////////////////////////////
//
// Spi.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

//
// These are the Winsock functions that this particular IFS LSP wishes to intercept.
// In this case, the LSP is just acting as a TCP proxy so it is only interested in the
// following functions.
//

void
FreeLspProviders(
        PROVIDER   *lspProvider,
        int         lspProviderCount,
        int        *lpErrno
        );

void
FindDestinationAddress( 
        SOCKET_CONTEXT *context, 
        const SOCKADDR *destAddr, 
        int             destLen,
        SOCKADDR      **proxyAddr, 
        int            *proxyLen
        );


int WSPAPI 
WSPCloseSocket(
        SOCKET s,        
        LPINT  lpErrno
        );

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
        );

int WSPAPI 
WSPGetPeerName(  
        SOCKET          s,
        struct sockaddr FAR * name,
        LPINT           namelen,
        LPINT           lpErrno
        );

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
        );

SOCKET WSPAPI
WSPSocket(
        int                 af,
        int                 type,
        int                 protocol,
        LPWSAPROTOCOL_INFOW lpProtocolInfo,
        GROUP               g,
        DWORD               dwFlags,
        LPINT               lpErrno
        );

////////////////////////////////////////////////////////////////////////////////
//
// Sockinfo.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Looks up the SOCKET_CONTEXT structure belonging to the given socket handle
SOCKET_CONTEXT *
FindSocketContext(
        SOCKET  s,
        BOOL    Remove = FALSE
        );

// Allocates a SOCKET_CONTEXT structure, initializes it, and inserts into the provider list
SOCKET_CONTEXT *
CreateSocketContext(
        PROVIDER  *Provider, 
        SOCKET     Socket, 
        int       *lpErrno
        );

// Frees a previously allocated SOCKET_CONTEXT structure
void 
FreeSocketContext(
        PROVIDER       *Provider,
        SOCKET_CONTEXT *Context
        );

// Frees all socket context strcutures belonging to the provider
void
FreeSocketContextList(
        PROVIDER   *Provider
        );

////////////////////////////////////////////////////////////////////////////////
//
// Extension.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

BOOL PASCAL FAR 
ExtConnectEx(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    );

////////////////////////////////////////////////////////////////////////////////
//
// External variable definitions
//
////////////////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION gCriticalSection;   // Critical section for initialization and 
extern INT              gLayerCount;        // Number of layered protocol entries for LSP
extern PROVIDER        *gLayerInfo;         // Provider structures for each layered protocol
extern WSPUPCALLTABLE   gMainUpCallTable;   // Upcall functions given to us by Winsock
extern GUID             gProviderGuid;      // GUID of our dummy hidden entry

#endif
