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

#define WM_SOCKET ( WM_USER + 321 )     // Message for our async window events

//
// This is the socket context data that we associate with each socket
//  that is passed back to the user app. That way when another API
//  is called we can query for this context information and find out
//  the socket handle to the lower provider.
//
typedef struct _SOCK_INFO
{
    SOCKET ProviderSocket;      // lower provider socket handle
    SOCKET LayeredSocket;       // app's socket handle
    DWORD  dwOutstandingAsync;  // count of outstanding async operations
    BOOL   bClosing;            // has the app closed the socket?

    volatile LONG  RefCount;    // How many threads are accessing this info?

    ULONGLONG  BytesSent;       // Byte counts
    ULONGLONG  BytesRecv;
    HANDLE     hIocp;           // associated with an IOCP?
    
    int    LastError;           // Last error that occured on this socket

    HWND   hWnd;                // Window (if any) associated with socket
    UINT   uMsg;                // Message for socket events

    CRITICAL_SECTION   SockCritSec; // Synchronize access to this object

    PROVIDER          *Provider;// Pointer to the provider from which socket was created

    LIST_ENTRY         Link;

} SOCK_INFO;

//
// Structure for mapping upper layer sockets to lower provider sockets in WSPSelect
//
typedef struct _FD_MAP
{
    SOCKET      ClientSocket;       // Upper layer socket handle
    SOCKET      ProvSocket;         // Lower layer socket handle
    SOCK_INFO  *Context;            // Pointer to the socket context
} FD_MAP;

////////////////////////////////////////////////////////////////////////////////
//
// Structures for each overlapped enabled API containing the arguments to that
//    function.
//
////////////////////////////////////////////////////////////////////////////////

// Argument list for the AcceptEx API
typedef struct _ACCEPTEXARGS
{
    SOCKET       sAcceptSocket,
                 sProviderAcceptSocket;
    PVOID        lpOutputBuffer;
    DWORD        dwReceiveDataLength,
                 dwLocalAddressLength,
                 dwRemoteAddressLength;
    DWORD        dwBytesReceived;
} ACCEPTEXARGS;

// Argument list for the TransmitFile API
typedef struct _TRANSMITFILEARGS
{
    HANDLE        hFile;
    DWORD         nNumberOfBytesToWrite,
                  nNumberOfBytesPerSend;
    LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers;
    DWORD         dwFlags;
} TRANSMITFILEARGS;

// Argument list for the ConnectEx API
typedef struct _CONNECTEXARGS
{
    SOCKET           s;
    SOCKADDR_STORAGE name;
    int              namelen;
    PVOID            lpSendBuffer;
    DWORD            dwSendDataLength;
    DWORD            dwBytesSent;
} CONNECTEXARGS;

// Argument list for TransmitPackets API
typedef struct _TRANSMITPACKETSARGS
{
    SOCKET          s;
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray;
    DWORD           nElementCount;
    DWORD           nSendSize;
    DWORD           dwFlags;
} TRANSMITPACKETSARGS;

// Argument list for DisconnectEx API
typedef struct _DISCONNECTEXARGS
{
    SOCKET          s;
    DWORD           dwFlags;
    DWORD           dwReserved;
} DISCONNECTEXARGS;

// Argument list for WSARecvMsg API
typedef struct _WSARECVMSGARGS
{
    WSAMSG          WsaMsg;
    DWORD           dwNumberOfBytesRecvd;
} WSARECVMSGARGS;

// Argument list for the WSARecv API
typedef struct _RECVARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesRecvd,
                   dwFlags;
} RECVARGS;

// Argument list for the WSARecvFrom API
typedef struct _RECVFROMARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesRecvd,
                   dwFlags;
    LPSOCKADDR     lpFrom;
    LPINT          lpFromLen;
} RECVFROMARGS;

// Argument list for the WSASend API
typedef struct _SENDARGS
{
    LPWSABUF       lpBuffers;
    DWORD          dwBufferCount;
    DWORD          dwNumberOfBytesSent;
    DWORD          dwFlags;
} SENDARGS;

// Argument list for the WSASendTo API
typedef struct _SENDTOARGS
{
    LPWSABUF         lpBuffers;
    DWORD            dwBufferCount;
    DWORD            dwNumberOfBytesSent;
    DWORD            dwFlags;
    SOCKADDR_STORAGE To;
    int              iToLen;
} SENDTOARGS;

// Argument list for the WSASendMsg API
typedef struct _WSASENDMSGARGS
{
    WSASENDMSG      SendMsg;
    WSAMSG          WsaMsg;
    DWORD           dwNumberOfBytesSent;
} WSASENDMSGARGS;

// Argument list for the WSAIoctl API
typedef struct _IOCTLARGS
{
    DWORD          dwIoControlCode;
    LPVOID         lpvInBuffer;
    DWORD          cbInBuffer;
    LPVOID         lpvOutBuffer;
    DWORD          cbOutBuffer;
    DWORD          cbBytesReturned;
} IOCTLARGS;

// Enumerated type of all asynchronous Winsock operations
typedef enum
{
    LSP_OP_IOCTL         = 1,
    LSP_OP_RECV,
    LSP_OP_RECVFROM,
    LSP_OP_SEND,
    LSP_OP_SENDTO,
    LSP_OP_TRANSMITFILE,
    LSP_OP_ACCEPTEX,
    LSP_OP_CONNECTEX,
    LSP_OP_DISCONNECTEX,
    LSP_OP_TRANSMITPACKETS,
    LSP_OP_RECVMSG,
    LSP_OP_SENDMSG
} LspOperation;

//
// Our OVERLAPPED structure that includes state and argument
//  information. This structure is used for all overlapped IO
//  operations. Whenever the user app requests overlapped IO
//  we intercept this and make our own call with our own 
//  structure. This way we will receive notification first
//  at which time we can perform post processesing. When we
//  are done we can then post the completion to the user app.
//
typedef struct _WSAOVERLAPPEDPLUS
{
    WSAOVERLAPPED   ProviderOverlapped;     // Overlap to pass to lower layer
    PROVIDER       *Provider;               // Reference to provider who owns this socket
    SOCK_INFO      *SockInfo;               // Socket initiating this operation
    SOCKET          CallerSocket;           // Upper layer's socket handle
    SOCKET          ProviderSocket;         // Lower layer's socket handle
    HANDLE          Iocp;                   
    int             Error;                  // Error that operation completed with

    BOOL            CloseThread;            // Indicates whether we need to close thread we opened

    union 
    {
        // Various arguments to the call being made
        ACCEPTEXARGS        AcceptExArgs;
        TRANSMITFILEARGS    TransmitFileArgs;
        CONNECTEXARGS       ConnectExArgs;
        TRANSMITPACKETSARGS TransmitPacketsArgs;
        DISCONNECTEXARGS    DisconnectExArgs;
        WSARECVMSGARGS      RecvMsgArgs;
        RECVARGS            RecvArgs;
        RECVFROMARGS        RecvFromArgs;
        SENDARGS            SendArgs;
        SENDTOARGS          SendToArgs;
        IOCTLARGS           IoctlArgs;
        WSASENDMSGARGS      SendMsgArgs;
    };

    LspOperation            Operation;              // Type of operation posted
    WSATHREADID             CallerThreadId;         // Which thread initiated operation
    LPWSAOVERLAPPED         lpCallerOverlapped;     // Upper layer's overlapped structure
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCallerCompletionRoutine;   // APC to queue upon completion

    LIST_ENTRY              Link;           // Linked list entry

} WSAOVERLAPPEDPLUS, * LPWSAOVERLAPPEDPLUS;

////////////////////////////////////////////////////////////////////////////////
//
// AsyncSelect.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Retrieves the async worker window and creates it if it hasn't already been
HWND 
GetWorkerWindow(
    );

// Issues the shutdown command to thread handling window messages
int 
StopAsyncWindowManager(
    );


////////////////////////////////////////////////////////////////////////////////
//
// Spi.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

//
// These are the Winsock functions an LSP must implement if it is non-IFS
//

SOCKET WSPAPI 
WSPAccept(
    SOCKET          s,                      
    struct sockaddr FAR * addr,  
    LPINT           addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD_PTR       dwCallbackData,          
    LPINT           lpErrno
    );

int WSPAPI 
WSPAddressToString(
    LPSOCKADDR          lpsaAddress,            
    DWORD               dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR              lpszAddressString,            
    LPDWORD             lpdwAddressStringLength,   
    LPINT               lpErrno
    );

int WSPAPI 
WSPAsyncSelect(
    SOCKET       s,
    HWND         hWnd,
    unsigned int wMsg,
    long         lEvent,
    LPINT        lpErrno
    );

int WSPAPI 
WSPBind(
    SOCKET                s,
    const struct sockaddr FAR * name,
    int                   namelen,
    LPINT                 lpErrno
    );

int WSPAPI 
WSPCancelBlockingCall(
    LPINT lpErrno
    );

int WSPAPI 
WSPCleanup(
    LPINT lpErrno  
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
WSPDuplicateSocket(
    SOCKET              s,
    DWORD               dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT               lpErrno
    );

int WSPAPI 
WSPEnumNetworkEvents(  
    SOCKET             s,
    WSAEVENT           hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT              lpErrno
    );

int WSPAPI 
WSPEventSelect(
    SOCKET   s,
    WSAEVENT hEventObject,
    long     lNetworkEvents,
    LPINT    lpErrno
    );

BOOL WSPAPI 
WSPGetOverlappedResult(
    SOCKET          s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD         lpcbTransfer,
    BOOL            fWait,
    LPDWORD         lpdwFlags,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetPeerName(  
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetSockName(
    SOCKET          s,
    struct sockaddr FAR * name,
    LPINT           namelen,
    LPINT           lpErrno
    );

int WSPAPI 
WSPGetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    char FAR * optval,
    LPINT      optlen,
    LPINT      lpErrno
    );

BOOL WSPAPI 
WSPGetQOSByName(
    SOCKET   s,
    LPWSABUF lpQOSName,
    LPQOS    lpQOS,
    LPINT    lpErrno
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
    );

int WSPAPI 
WSPListen(
    SOCKET s,        
    int    backlog,     
    LPINT  lpErrno
    );

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
    );

int WSPAPI 
WSPRecvDisconnect(
    SOCKET   s,
    LPWSABUF lpInboundDisconnectData,
    LPINT    lpErrno
    );

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
    );

int WSPAPI 
WSPSelect(
    int          nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT        lpErrno
    );

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
    );

int WSPAPI 
WSPSendDisconnect(
    SOCKET   s,
    LPWSABUF lpOutboundDisconnectData,
    LPINT    lpErrno
    );

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
    );

int WSPAPI 
WSPSetSockOpt(
    SOCKET     s,
    int        level,
    int        optname,
    const char FAR * optval,   
    int        optlen,
    LPINT      lpErrno
    );

int WSPAPI 
WSPShutdown (
    SOCKET s,
    int    how,
    LPINT  lpErrno
    );

int WSPAPI 
WSPStringToAddress(
    LPWSTR              AddressString,
    INT                 AddressFamily,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR          lpAddress,
    LPINT               lpAddressLength,
    LPINT               lpErrno
    );

SOCKET WSPAPI 
WSPSocket(
    int                 af,
    int                 type,
    int                 protocol,
    __in LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP               g,
    DWORD               dwFlags,
    LPINT               lpErrno
    );

// Copies the offset values from one overlapped structure to another
void 
CopyOffset(
    WSAOVERLAPPED  *ProviderOverlapped, 
    WSAOVERLAPPED  *UserOverlapped
    );

// Creates a new WSABUF array and copies the buffer and length values over
WSABUF *
CopyWSABuf(
    WSABUF *BufferArray, 
    DWORD   BufferCount,
    int    *lpErrno
    );

// Frees a previously allocated WSABUF array
void 
FreeWSABuf(
    WSABUF *BufferArray
    );

////////////////////////////////////////////////////////////////////////////////
//
// Overlap.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Initialize the overlapped system for handing asynchronous (overlapped) I/O
int 
InitOverlappedManager(
    );

// Issues the shutdown command for all worker threads to exit
int 
StopOverlappedManager(
    );

// Queue an overlapped operation for execution
int 
QueueOverlappedOperation(
    WSAOVERLAPPEDPLUS  *lpOverlapped, 
    SOCK_INFO          *Context
    );

// Allocate and initialize a WSAOVERLAPPEDPLUS structure which describes an overlapped operation
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
    );

// This function handles the completion of an overlapped operation
void CALLBACK 
IntermediateCompletionRoutine(
    DWORD           dwError, 
    DWORD           cbTransferred,
    LPWSAOVERLAPPED lpOverlapped, 
    DWORD           dwFlags
    );

// If an overlapped operation fails inline, we must undo some state
void
UndoOverlappedOperation( 
    SOCK_INFO         *SocketContext,
    WSAOVERLAPPEDPLUS *ProviderOverlapped
    );

// Frees all cached WSAOVERLAPPEDPLUS structures when the LSP is unloaded
void
FreeOverlappedLookasideList(
    );

////////////////////////////////////////////////////////////////////////////////
//
// Sockinfo.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

SOCK_INFO *
GetCallerSocket(
    PROVIDER   *provider, 
    SOCKET      ProviderSocket
    );

// Allocates a SOCK_INFO structure, initializes it, and inserts into the provider list
SOCK_INFO *
CreateSockInfo(
    PROVIDER   *Provider, 
    SOCKET      ProviderSocket, 
    SOCK_INFO  *Inherit, 
    BOOL        Insert,
    int        *lpErrno
    );

// Looks up the socket context structure associated with the application socket
SOCK_INFO *
FindAndRefSocketContext(
    SOCKET  s, 
    int    *err
    );

// Decrements the reference count on the given socket context object
void 
DerefSocketContext(
    SOCK_INFO  *context, 
    int        *err
    );

// Frees a previously allocated SOCK_INFO structure
void 
FreeSockInfo(
    SOCK_INFO *info
    );

// Inserts the SOCK_INFO structure at the tail of the given provider's socket list
void 
InsertSocketInfo(
    PROVIDER   *provider, 
    SOCK_INFO  *sock
    );

// Removes the given SOCK_INFO structure from the provider's socket list
void 
RemoveSocketInfo(
    PROVIDER   *provider, 
    SOCK_INFO  *sock
    );

// Enters the SOCK_INFO structure's critical section preventing other threads from accessing it
void 
AcquireSocketLock(
    SOCK_INFO  *SockInfo
    );

// Releases the SOCK_INFO structure's critical section
void 
ReleaseSocketLock(
    SOCK_INFO  *SockInfo
    );

// Closes all the sockets and frees all resources associated with a provider
void 
CloseAndFreeSocketInfo(
    PROVIDER   *provider,
    BOOL        processDetach
    );


////////////////////////////////////////////////////////////////////////////////
//
// Extension.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

BOOL PASCAL FAR 
ExtTransmitFile (
    IN SOCKET hSocket,
    IN HANDLE hFile,
    IN DWORD nNumberOfBytesToWrite,
    IN DWORD nNumberOfBytesPerSend,
    IN LPOVERLAPPED lpOverlapped,
    IN LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN DWORD dwReserved
    );

BOOL PASCAL FAR 
ExtAcceptEx(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

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

BOOL PASCAL FAR 
ExtTransmitPackets(
    SOCKET hSocket,
    LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,
    DWORD nElementCount,
    DWORD nSendSize,
    LPOVERLAPPED lpOverlapped,
    DWORD dwFlags
    );

BOOL PASCAL FAR 
ExtDisconnectEx(
    IN SOCKET s,
    IN LPOVERLAPPED lpOverlapped,
    IN DWORD  dwFlags,
    IN DWORD  dwReserved
    );

INT PASCAL FAR 
ExtWSARecvMsg(
    IN SOCKET s,
    IN OUT LPWSAMSG lpMsg,
    OUT LPDWORD lpdwNumberOfBytesRecvd,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

INT PASCAL FAR
ExtWSASendMsg(
    IN SOCKET s,
    IN WSASENDMSG *sendMsg,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

INT PASCAL FAR
ExtWSAPoll(
    IN SOCKET s,
    IN WSAPOLLDATA *pollData,
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    );

// Loads the given extension function from the lower provider
BOOL
LoadExtensionFunction(
    FARPROC   **func,
    GUID        ExtensionGuid,
    LPWSPIOCTL  fnIoctl,
    SOCKET      s
    );

////////////////////////////////////////////////////////////////////////////////
//
// External variable definitions
//
////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE        gDllInstance;       // Instance passed to DllMain
extern CRITICAL_SECTION gCriticalSection,   // Critical section for initialization and 
                                            //    socket list manipulation
                        gOverlappedCS;      // Used in overlapped IO handling
extern INT              gLayerCount;        // Number of layered protocol entries for LSP
extern PROVIDER        *gBaseInfo;          // Provider structures for each layered protocol
extern HANDLE           gAddContextEvent,   // Event signaled whenver new socket context 
                                            //    is added to a PROVIDER
                        gIocp;              // Completion port handle
extern WSPUPCALLTABLE   gMainUpCallTable;   // Upcall functions given to us by Winsock
extern GUID             gProviderGuid;      // GUID of our dummy hidden entry

#endif
