// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: common.h
//
// Description:
//             This file contains the common definitions for various
// constants and functions used in the server modules.

#ifndef __ACCEPT_COMMON_H__
#define __ACCEPT_COMMON_H__

#include <winsock2.h>   // for Winsock API
#include <windows.h>    // for Win32 APIs and types
#include <ws2tcpip.h>   // for IPv6 support
#include <wspiapi.h>    // for IPv6 support
#include <strsafe.h>    // for safe versions of string functions
#include <stdio.h>      // for printing to stdout.

// This is the number of clients that can be accomodated in the FD_SETs that
// are used in the select command. In spite of the name referring to the
// max number of clients, the actual number of clients that will be
// accomodated will be a little less because the select will also include the
// server interfaces on which the app is listening.
// If you want to increase this value, first undefine FD_SETSIZE and redefine
// FD_SETSIZE to the required value before including winsock2.h
#define MAX_CLIENTS             FD_SETSIZE

// This is the default size of the data buffer that'll be used in recv
// each time. In real apps, this value will be much higher.
#define RECV_DATA_SIZE          4096


// Different types of accepts that are supported by the server.

// This flag indicates the server to use select() and do a 
// non-blocking accept with single thread.
const BYTE NON_BLOCKING_ACCEPT = 1;

// This flag indicates the server to use WSAAsyncSelect() to get 
// Window messages and call accept on receiving FD_ACCEPT event.
const BYTE ASYNC_SELECT_ACCEPT = 2;

// This is the message that'll be sent to the server by winsock if 
// any of the registered socket events happen in the AsyncSelect case.
// Any value above the WM_USER can be used to avoid collision with the standard
// Windows messages.
#define WM_USER_ASYNCSELECT_MSG     WM_USER + 1


// default values for various command-line options

// The default address family to bind to.
#define DEFAULT_ADDRESS_FAMILY          AF_UNSPEC

// The default interface to listen to.
#define DEFAULT_INTERFACE               NULL

// The default port to listen to.
#define DEFAULT_PORT                    "7243"

// The default type of accept to be done.
#define DEFAULT_TYPE_OF_ACCEPT          NON_BLOCKING_ACCEPT


// This structure defines the contents of the data buffer that'll be used
// to store the messages that are received from and sent to the client.
typedef struct _DATA_BUFFER
{
    char buf[RECV_DATA_SIZE];  // the data buffer
    int  dataSize;             // length of the actual data present
    int  sendOffset;           // position of the next byte in buf to be sent
    BOOL isNewData;            // TRUE if buf has not been fully sent
} DATA_BUFFER;


// This structure holds the information for each socket the server creates.
// The server maintains a list of these structures and operates on them.
typedef struct _SOCK_INFO
{
    SOCKET sock;                // socket handle
    BOOL   isSocketListening;   // TRUE if the socket is listening
    struct _SOCK_INFO *prev,    // previous structure in the list
                      *next;    // next structure in the list
    DATA_BUFFER recdData;       // details of data buffer that's recd/sent
    int     nTotalRecd;         // total number of bytes recd so far
    int     nTotalSent;         // total number of bytes sent so far
    BOOL    isFdCloseRecd;      // TRUE if FD_CLOSE event was received
} SOCK_INFO, *PSOCK_INFO;


// This structure bundles all the global variables needed between 
// different functions into a global context.
typedef struct _AcceptContext
{
    BYTE    addressFamily;      // Address Family requested
    char    *szInterface;       // Interface to listen on
    char    *szPort;            // Port to listen on
    BYTE    typeOfAccept;       // Which type of accept should be done
    SOCK_INFO *pSockList;       // List of the listening/accepted sockets
    HWND    hAcceptWindow;      // Handle to the hidden accept window
} AcceptContext;

// This is the only global variable that'll be used throughout the server
// module.
extern AcceptContext g_AcceptContext;

// functions defined in SockInfo.cpp
PSOCK_INFO AllocAndInitSockInfo();
void FreeSockInfo(PSOCK_INFO pSockInfo);
void AddSockInfoToList(PSOCK_INFO *ppHead, PSOCK_INFO pNewSockInfo);
void DeleteSockInfoFromList(PSOCK_INFO *ppHead, PSOCK_INFO pDelSockInfo);

// functions exported from Common.cpp
void PrintAddressString(LPSOCKADDR pSockAddr, DWORD dwSockAddrLen);
void CreateListeningSockets();
void DestroyListeningSockets();
PSOCK_INFO ProcessAcceptEvent(PSOCK_INFO pSockInfo);
BOOL ProcessReadEvent(PSOCK_INFO pSockInfo);
BOOL SendData(PSOCK_INFO pSockInfo);

// functions exported from NBAccept.cpp
void NonBlockingAcceptMain();

// functions exported from ASAccept.cpp
void AsyncSelectAcceptMain();

#endif // __ACCEPT_COMMON_H__
