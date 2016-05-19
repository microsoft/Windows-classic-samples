// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      iocpserver.cpp
//
// Abstract:
//      This program is a Winsock echo server program that uses I/O Completion Ports 
//      (IOCP) to receive data from and echo data back to a sending client. The server 
//      program supports multiple clients connecting via TCP/IP and sending arbitrary 
//      sized data buffers which the server then echoes back to the client.  For 
//      convenience a simple client program, iocpclient was developed to connect 
//      and continually send data to the server to stress it.
//
//      Direct IOCP support was added to Winsock 2 and is fully implemented on the NT 
//      platform.  IOCPs provide a model for developing very high performance and very 
//      scalable server programs.
//
//      The basic idea is that this server continuously accepts connection requests from 
//      a client program.  When this happens, the accepted socket descriptor is added to 
//      the existing IOCP and an initial receive (WSARecv) is posted on that socket.  When 
//      the client then sends data on that socket, a completion packet will be delivered 
//      and handled by one of the server's worker threads.  The worker thread echoes the 
//      data back to the sender by posting a send (WSASend) containing all the data just 
//      received.  When sending the data back to the client completes, another completion
//      packet will be delivered and again handled by one of the server's worker threads.  
//      Assuming all the data that needed to be sent was actually sent, another receive 
//      (WSARecv) is once again posted and the scenario repeats itself until the client 
//      stops sending data.
//
//      When using IOCPs it is important to remember that the worker threads must be able
//      to distinguish between I/O that occurs on multiple handles in the IOCP as well as 
//      multiple I/O requests initiated on a single handle.  The per handle data 
//      (PER_SOCKET_CONTEXT) is associated with the handle as the CompletionKey when the 
//      handle is added to the IOCP using CreateIoCompletionPort.  The per IO operation 
//      data (PER_IO_CONTEXT) is associated with a specific handle during an I/O 
//      operation as part of the overlapped structure passed to either WSARecv or 
//      WSASend.  Please notice that the first member of the PER_IO_CONTEXT structure is 
//      a WSAOVERLAPPED structure (compatible with the Win32 OVERLAPPED structure).  
//
//      When the worker thread unblocks from GetQueuedCompletionStatus, the key 
//      associated with the handle when the handle was added to the IOCP is returned as 
//      well as the overlapped structure associated when this particular I/O operation 
//      was initiated.
//      
//      This program cleans up all resources and shuts down when CTRL-C is pressed.  
//      This will cause the main thread to break out of the accept loop and close all open 
//      sockets and free all context data.  The worker threads get unblocked by posting  
//      special I/O packets with a NULL CompletionKey to the IOCP.  The worker 
//      threads check for a NULL CompletionKey and exits if it encounters one. If CTRL-BRK 
//      is pressed instead, cleanup process is same as above but instead of exit the process, 
//      the program loops back to restart the server.

//      Another point worth noting is that the Win32 API CreateThread() does not 
//      initialize the C Runtime and therefore, C runtime functions such as 
//      printf() have been avoid or rewritten (see myprintf()) to use just Win32 APIs.
//
//  Usage:
//      Start the server and wait for connections on port 6001
//          iocpserver -e:6001
//
//  Build:
//      Use the headers and libs from the April98 Platform SDK or later.
//      Link with ws2_32.lib
//      
//
//

#pragma warning (disable:4127)

#ifdef _IA64_
	#pragma warning(disable:4267)
#endif 

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p)   HeapFree(GetProcessHeap(),0,(p))

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

#include "iocpserver.h"

char *g_Port = DEFAULT_PORT;
BOOL g_bEndServer = FALSE;			// set to TRUE on CTRL-C
BOOL g_bRestart = TRUE;				// set to TRUE to CTRL-BRK
BOOL g_bVerbose = FALSE;
DWORD g_dwThreadCount = 0;		//worker thread count
HANDLE g_hIOCP = INVALID_HANDLE_VALUE;
SOCKET g_sdListen = INVALID_SOCKET;
HANDLE g_ThreadHandles[MAX_WORKER_THREAD];
PPER_SOCKET_CONTEXT g_pCtxtList = NULL;		// linked list of context info structures
											// maintained to allow the the cleanup 
											// handler to cleanly close all sockets and 
											// free resources.

CRITICAL_SECTION g_CriticalSection;		// guard access to the global context list

int myprintf(const char *lpFormat, ...);

void __cdecl main (int argc, char *argv[]) {

	SYSTEM_INFO systemInfo;
	WSADATA wsaData;
	SOCKET sdAccept = INVALID_SOCKET;
	PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;
	DWORD dwRecvNumBytes = 0;     
	DWORD dwFlags = 0;            
	int nRet = 0;

	for( int i = 0; i < MAX_WORKER_THREAD; i++ ) {
		g_ThreadHandles[i] = INVALID_HANDLE_VALUE;
	}

	if( !ValidOptions(argc, argv) )
		return;

	if( !SetConsoleCtrlHandler(CtrlHandler, TRUE) ) {
		myprintf("SetConsoleCtrlHandler() failed to install console handler: %d\n", 
				 GetLastError());
		return;
	}

	GetSystemInfo(&systemInfo);
	g_dwThreadCount = systemInfo.dwNumberOfProcessors * 2;

	if( (nRet = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0 ) {
		myprintf("WSAStartup() failed: %d\n",nRet);
		SetConsoleCtrlHandler(CtrlHandler, FALSE);
		return;
	}

	__try
    {
        InitializeCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("InitializeCriticalSection raised exception.\n");
        return;

    }

	while( g_bRestart ) {
		g_bRestart = FALSE;
		g_bEndServer = FALSE;

		__try {
			g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
			if( g_hIOCP == NULL ) {
				myprintf("CreateIoCompletionPort() failed to create I/O completion port: %d\n", 
						GetLastError());
				__leave;
			}

			for( DWORD dwCPU = 0; dwCPU < g_dwThreadCount; dwCPU++ ) {

				//
				// Create worker threads to service the overlapped I/O requests.  The decision
				// to create 2 worker threads per CPU in the system is a heuristic.  Also,
				// note that thread handles are closed right away, because we will not need them
				// and the worker threads will continue to execute.
				//
				HANDLE hThread = INVALID_HANDLE_VALUE;
				DWORD dwThreadId = 0;

				hThread = CreateThread(NULL, 0, WorkerThread, g_hIOCP, 0, &dwThreadId);
				if( hThread == NULL ) {
					myprintf("CreateThread() failed to create worker thread: %d\n", 
							GetLastError());
					__leave;
				}
				g_ThreadHandles[dwCPU] = hThread;
				hThread = INVALID_HANDLE_VALUE;
			}

			if( !CreateListenSocket() )
				__leave;

			while( TRUE ) {

				//
				// Loop forever accepting connections from clients until console shuts down.
				//
				sdAccept = WSAAccept(g_sdListen, NULL, NULL, NULL, 0);
				if( sdAccept == SOCKET_ERROR ) {

					//
					// If user hits Ctrl+C or Ctrl+Brk or console window is closed, the control
					// handler will close the g_sdListen socket. The above WSAAccept call will 
					// fail and we thus break out the loop,
					//
					myprintf("WSAAccept() failed: %d\n", WSAGetLastError());
					__leave;
				}

                //
				// we add the just returned socket descriptor to the IOCP along with its
				// associated key data.  Also the global list of context structures
				// (the key data) gets added to a global list.
				//
				lpPerSocketContext = UpdateCompletionPort(sdAccept, ClientIoRead, TRUE);
				if( lpPerSocketContext == NULL )
					__leave;

				//
				// if a CTRL-C was pressed "after" WSAAccept returns, the CTRL-C handler
				// will have set this flag and we can break out of the loop here before
				// we go ahead and post another read (but after we have added it to the 
				// list of sockets to close).
				//
				if( g_bEndServer )
					break;

                //
				// post initial receive on this socket
				//
				nRet = WSARecv(sdAccept, &(lpPerSocketContext->pIOContext->wsabuf), 
							   1, &dwRecvNumBytes, &dwFlags,
							   &(lpPerSocketContext->pIOContext->Overlapped), NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					myprintf("WSARecv() Failed: %d\n", WSAGetLastError());
					CloseClient(lpPerSocketContext, FALSE);
				}
			} //while
		}

		__finally   {

			g_bEndServer = TRUE;

            //
			// Cause worker threads to exit
			//
			if( g_hIOCP ) {
				for( DWORD i = 0; i < g_dwThreadCount; i++ )
					PostQueuedCompletionStatus(g_hIOCP, 0, 0, NULL);
			}
            
			//
			//Make sure worker threads exits.
            //
			if( WAIT_OBJECT_0 != WaitForMultipleObjects( g_dwThreadCount,  g_ThreadHandles, TRUE, 1000) )
				myprintf("WaitForMultipleObjects() failed: %d\n", GetLastError());
			else
				for( DWORD i = 0; i < g_dwThreadCount; i++ ) {
					if( g_ThreadHandles[i] != INVALID_HANDLE_VALUE ) CloseHandle(g_ThreadHandles[i]);
					g_ThreadHandles[i] = INVALID_HANDLE_VALUE;
				}

			CtxtListFree();

			if( g_hIOCP ) {
				CloseHandle(g_hIOCP);
				g_hIOCP = NULL;
			}

			if( g_sdListen != INVALID_SOCKET ) {
				closesocket(g_sdListen); 
				g_sdListen = INVALID_SOCKET;
			}

			if( sdAccept != INVALID_SOCKET ) {
				closesocket(sdAccept); 
				sdAccept = INVALID_SOCKET;
			}

		} //finally

		if( g_bRestart ) {
			myprintf("\niocpserver is restarting...\n");
		} else
			myprintf("\niocpserver is exiting...\n");

	} //while (g_bRestart)

	DeleteCriticalSection(&g_CriticalSection);
	WSACleanup();
	SetConsoleCtrlHandler(CtrlHandler, FALSE);
} //main      

//
//  Just validate the command line options.
//
BOOL ValidOptions(int argc, char *argv[]) {

	BOOL bRet = TRUE;

	for( int i = 1; i < argc; i++ ) {
		if( (argv[i][0] =='-') || (argv[i][0] == '/') ) {
			switch( tolower(argv[i][1]) ) {
			case 'e':
				if( strlen(argv[i]) > 3 )
					g_Port = &argv[i][3];
				break;

			case 'v':
				g_bVerbose = TRUE;
				break;

			case '?':
				myprintf("Usage:\n  iocpserver [-p:port] [-v] [-?]\n");
				myprintf("  -e:port\tSpecify echoing port number\n");        
				myprintf("  -v\t\tVerbose\n");        
				myprintf("  -?\t\tDisplay this help\n");
				bRet = FALSE;
				break;

			default:
				myprintf("Unknown options flag %s\n", argv[i]);
				bRet = FALSE;
				break;
			}
		}
	}   

	return(bRet);
}

//
//  Intercept CTRL-C or CTRL-BRK events and cause the server to initiate shutdown.
//  CTRL-BRK resets the restart flag, and after cleanup the server restarts.
//
BOOL WINAPI CtrlHandler (DWORD dwEvent) {

	SOCKET sockTemp = INVALID_SOCKET;

	switch( dwEvent ) {
	case CTRL_BREAK_EVENT: 
		g_bRestart = TRUE;
	case CTRL_C_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_CLOSE_EVENT:
		if( g_bVerbose )
			myprintf("CtrlHandler: closing listening socket\n");

		//
		// cause the accept in the main thread loop to fail
		//

        //
		//We want to make closesocket the last call in the handler because it will
		//cause the WSAAccept to return in the main thread
		//
		sockTemp = g_sdListen;
		g_sdListen = INVALID_SOCKET;
		g_bEndServer = TRUE;
		closesocket(sockTemp);
		sockTemp = INVALID_SOCKET;
		break;

	default:
		// unknown type--better pass it on.
		return(FALSE);
	}
	return(TRUE);
}

//
//  Create a listening socket.
//
BOOL CreateListenSocket(void) {

	int nRet = 0;
	int nZero = 0;
	struct addrinfo hints = {0};
	struct addrinfo *addrlocal = NULL;

	//
	// Resolve the interface
	//
    hints.ai_flags  = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;

	if( getaddrinfo(NULL, g_Port, &hints, &addrlocal) != 0 ) {
		myprintf("getaddrinfo() failed with error %d\n", WSAGetLastError());
        return(FALSE);
	}

	if( addrlocal == NULL ) {
		myprintf("getaddrinfo() failed to resolve/convert the interface\n");
        return(FALSE);
	}

	g_sdListen = WSASocket(addrlocal->ai_family, addrlocal->ai_socktype, addrlocal->ai_protocol, 
						   NULL, 0, WSA_FLAG_OVERLAPPED); 
	if( g_sdListen == INVALID_SOCKET ) {
		myprintf("WSASocket(g_sdListen) failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

	nRet = bind(g_sdListen, addrlocal->ai_addr, (int) addrlocal->ai_addrlen);
	if( nRet == SOCKET_ERROR ) {
		myprintf("bind() failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

	nRet = listen(g_sdListen, 5);
	if( nRet == SOCKET_ERROR ) {
		myprintf("listen() failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

    //
	// Disable send buffering on the socket.  Setting SO_SNDBUF
	// to 0 causes winsock to stop buffering sends and perform
	// sends directly from our buffers, thereby reducing CPU usage.
    //
    // However, this does prevent the socket from ever filling the
    // send pipeline. This can lead to packets being sent that are
    // not full (i.e. the overhead of the IP and TCP headers is 
    // great compared to the amount of data being carried).
    //
    // Disabling the send buffer has less serious repercussions 
    // than disabling the receive buffer.
	//
	nZero = 0;
	nRet = setsockopt(g_sdListen, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
	if( nRet == SOCKET_ERROR ) {
		myprintf("setsockopt(SNDBUF) failed: %d\n", WSAGetLastError());
		return(FALSE);
	}

    //
    // Don't disable receive buffering. This will cause poor network
    // performance since if no receive is posted and no receive buffers,
    // the TCP stack will set the window size to zero and the peer will
    // no longer be allowed to send data.
    //

    // 
    // Do not set a linger value...especially don't set it to an abortive
    // close. If you set abortive close and there happens to be a bit of
    // data remaining to be transfered (or data that has not been 
    // acknowledged by the peer), the connection will be forcefully reset
    // and will lead to a loss of data (i.e. the peer won't get the last
    // bit of data). This is BAD. If you are worried about malicious
    // clients connecting and then not sending or receiving, the server
    // should maintain a timer on each connection. If after some point,
    // the server deems a connection is "stale" it can then set linger
    // to be abortive and close the connection.
    //

    /*
	LINGER lingerStruct;

	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;

	nRet = setsockopt(g_sdListen, SOL_SOCKET, SO_LINGER,
					  (char *)&lingerStruct, sizeof(lingerStruct) );
	if( nRet == SOCKET_ERROR ) {
		myprintf("setsockopt(SO_LINGER) failed: %d\n", WSAGetLastError());
		return(FALSE);
	}
    */

	freeaddrinfo(addrlocal);

	return(TRUE);
}

//
// Worker thread that handles all I/O requests on any socket handle added to the IOCP.
//
DWORD WINAPI WorkerThread (LPVOID WorkThreadContext) {

	HANDLE hIOCP = (HANDLE)WorkThreadContext;
	BOOL bSuccess = FALSE;
	int nRet = 0;
	LPWSAOVERLAPPED lpOverlapped = NULL;
	PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;
	PPER_IO_CONTEXT lpIOContext = NULL; 
	WSABUF buffRecv;
	WSABUF buffSend;
	DWORD dwRecvNumBytes = 0;
	DWORD dwSendNumBytes = 0;
	DWORD dwFlags = 0;
	DWORD dwIoSize = 0;

	while( TRUE ) {

        //
		// continually loop to service io completion packets
		//
		bSuccess = GetQueuedCompletionStatus(hIOCP, &dwIoSize,
											 (PDWORD_PTR)&lpPerSocketContext,
											 (LPOVERLAPPED *)&lpOverlapped, 
											 INFINITE);
		if( !bSuccess )
			myprintf("GetQueuedCompletionStatus() failed: %d\n", GetLastError());

		if( lpPerSocketContext == NULL ) {

			//
			// CTRL-C handler used PostQueuedCompletionStatus to post an I/O packet with
			// a NULL CompletionKey (or if we get one for any reason).  It is time to exit.
			//
			return(0);
		}

		if( g_bEndServer ) {

			//
			// main thread will do all cleanup needed - see finally block
			//
			return(0);
		}

		if( !bSuccess || (bSuccess && (dwIoSize == 0)) ) {

			//
			// client connection dropped, continue to service remaining (and possibly 
			// new) client connections
			//
			CloseClient(lpPerSocketContext, FALSE); 
			continue;
		}

        //
		// determine what type of IO packet has completed by checking the PER_IO_CONTEXT 
		// associated with this socket.  This will determine what action to take.
		//
		lpIOContext = (PPER_IO_CONTEXT)lpOverlapped;
		switch( lpIOContext->IOOperation ) {
		case ClientIoRead:

			//
			// a read operation has completed, post a write operation to echo the
			// data back to the client using the same data buffer.
			//
			lpIOContext->IOOperation = ClientIoWrite;
			lpIOContext->nTotalBytes = dwIoSize;
			lpIOContext->nSentBytes  = 0;
			lpIOContext->wsabuf.len  = dwIoSize;
			dwFlags = 0;
			nRet = WSASend(lpPerSocketContext->Socket, &lpIOContext->wsabuf, 1, 
						   &dwSendNumBytes, dwFlags, &(lpIOContext->Overlapped), NULL);
			if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
				myprintf("WSASend() failed: %d\n", WSAGetLastError());
				CloseClient(lpPerSocketContext, FALSE);
			} else if( g_bVerbose ) {
				myprintf("WorkerThread %d: Socket(%d) Recv completed (%d bytes), Send posted\n", 
					   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
			}
			break;

		case ClientIoWrite:

			//
			// a write operation has completed, determine if all the data intended to be
			// sent actually was sent.
			//
			lpIOContext->IOOperation = ClientIoWrite;
			lpIOContext->nSentBytes  += dwIoSize;
			dwFlags = 0;
			if( lpIOContext->nSentBytes < lpIOContext->nTotalBytes ) {

				//
				// the previous write operation didn't send all the data,
				// post another send to complete the operation
				//
				buffSend.buf = lpIOContext->Buffer + lpIOContext->nSentBytes;
				buffSend.len = lpIOContext->nTotalBytes - lpIOContext->nSentBytes;
				nRet = WSASend (lpPerSocketContext->Socket, &buffSend, 1, 
								&dwSendNumBytes, dwFlags, &(lpIOContext->Overlapped), NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					myprintf("WSASend() failed: %d\n", WSAGetLastError());
					CloseClient(lpPerSocketContext, FALSE);
				} else if( g_bVerbose ) {
					myprintf("WorkerThread %d: Socket(%d) Send partially completed (%d bytes), Recv posted\n", 
						   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
				}
			} else {

				//
				// previous write operation completed for this socket, post another recv
				//
				lpIOContext->IOOperation = ClientIoRead; 
				dwRecvNumBytes = 0;
				dwFlags = 0;
				buffRecv.buf = lpIOContext->Buffer,
				buffRecv.len = MAX_BUFF_SIZE;
				nRet = WSARecv(lpPerSocketContext->Socket, &buffRecv, 1, 
							   &dwRecvNumBytes, &dwFlags, &lpIOContext->Overlapped, NULL);
				if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
					myprintf("WSARecv() failed: %d\n", WSAGetLastError());
					CloseClient(lpPerSocketContext, FALSE);
				} else if( g_bVerbose ) {
					myprintf("WorkerThread %d: Socket(%d) Send completed (%d bytes), Recv posted\n", 
						   GetCurrentThreadId(), lpPerSocketContext->Socket, dwIoSize);
				}
			}
			break;

		} //switch
	} //while
	return(0);
} 

//
//  Allocate a context structures for the socket and add the socket to the IOCP.  
//  Additionally, add the context structure to the global list of context structures.
//
PPER_SOCKET_CONTEXT UpdateCompletionPort(SOCKET sd, IO_OPERATION ClientIo,
										 BOOL bAddToList) {

	PPER_SOCKET_CONTEXT lpPerSocketContext;

	lpPerSocketContext = CtxtAllocate(sd, ClientIo);
	if( lpPerSocketContext == NULL )
		return(NULL);

	g_hIOCP = CreateIoCompletionPort((HANDLE)sd, g_hIOCP, (DWORD_PTR)lpPerSocketContext, 0);
	if( g_hIOCP == NULL ) {
		myprintf("CreateIoCompletionPort() failed: %d\n", GetLastError());
		if( lpPerSocketContext->pIOContext )
			xfree(lpPerSocketContext->pIOContext);
		xfree(lpPerSocketContext);
		return(NULL);
	}

    //
	//The listening socket context (bAddToList is FALSE) is not added to the list.
	//All other socket contexts are added to the list.
	//
	if( bAddToList ) CtxtListAddTo(lpPerSocketContext);

	if( g_bVerbose )
		myprintf("UpdateCompletionPort: Socket(%d) added to IOCP\n", lpPerSocketContext->Socket);

	return(lpPerSocketContext);
}

//
//  Close down a connection with a client.  This involves closing the socket (when 
//  initiated as a result of a CTRL-C the socket closure is not graceful).  Additionally, 
//  any context data associated with that socket is free'd.
//
VOID CloseClient (PPER_SOCKET_CONTEXT lpPerSocketContext,
				  BOOL bGraceful) {

    __try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("EnterCriticalSection raised an exception.\n");
        return;
    }

	if( lpPerSocketContext ) {
		if( g_bVerbose )
			myprintf("CloseClient: Socket(%d) connection closing (graceful=%s)\n",
				   lpPerSocketContext->Socket, (bGraceful?"TRUE":"FALSE"));
		if( !bGraceful ) {

			//
			// force the subsequent closesocket to be abortative.
			//
			LINGER  lingerStruct;

			lingerStruct.l_onoff = 1;
			lingerStruct.l_linger = 0;
			setsockopt(lpPerSocketContext->Socket, SOL_SOCKET, SO_LINGER,
					   (char *)&lingerStruct, sizeof(lingerStruct) );
		}
		closesocket(lpPerSocketContext->Socket);
		lpPerSocketContext->Socket = INVALID_SOCKET;
		CtxtListDeleteFrom(lpPerSocketContext);
		lpPerSocketContext = NULL;
	} else {
		myprintf("CloseClient: lpPerSocketContext is NULL\n");
	}

    LeaveCriticalSection(&g_CriticalSection);

	return;    
} 

//
// Allocate a socket context for the new connection.  
//
PPER_SOCKET_CONTEXT CtxtAllocate(SOCKET sd, IO_OPERATION ClientIO) {

	PPER_SOCKET_CONTEXT lpPerSocketContext;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("EnterCriticalSection raised an exception.\n");
        return NULL;
    }

	lpPerSocketContext = (PPER_SOCKET_CONTEXT)xmalloc(sizeof(PER_SOCKET_CONTEXT));
	if( lpPerSocketContext ) {
		lpPerSocketContext->pIOContext = (PPER_IO_CONTEXT)xmalloc(sizeof(PER_IO_CONTEXT));
		if( lpPerSocketContext->pIOContext ) {
			lpPerSocketContext->Socket = sd;
			lpPerSocketContext->pCtxtBack = NULL;
			lpPerSocketContext->pCtxtForward = NULL;

			lpPerSocketContext->pIOContext->Overlapped.Internal = 0;
			lpPerSocketContext->pIOContext->Overlapped.InternalHigh = 0;
			lpPerSocketContext->pIOContext->Overlapped.Offset = 0;
			lpPerSocketContext->pIOContext->Overlapped.OffsetHigh = 0;
			lpPerSocketContext->pIOContext->Overlapped.hEvent = NULL;
			lpPerSocketContext->pIOContext->IOOperation = ClientIO;
			lpPerSocketContext->pIOContext->pIOContextForward = NULL;
			lpPerSocketContext->pIOContext->nTotalBytes = 0;
			lpPerSocketContext->pIOContext->nSentBytes  = 0;
			lpPerSocketContext->pIOContext->wsabuf.buf  = lpPerSocketContext->pIOContext->Buffer;
			lpPerSocketContext->pIOContext->wsabuf.len  = sizeof(lpPerSocketContext->pIOContext->Buffer);

			ZeroMemory(lpPerSocketContext->pIOContext->wsabuf.buf, lpPerSocketContext->pIOContext->wsabuf.len);
		} else {
			xfree(lpPerSocketContext);
			myprintf("HeapAlloc() PER_IO_CONTEXT failed: %d\n", GetLastError());
		}

	} else {
		myprintf("HeapAlloc() PER_SOCKET_CONTEXT failed: %d\n", GetLastError());
	}

	LeaveCriticalSection(&g_CriticalSection);

	return(lpPerSocketContext);
} 

//
//  Add a client connection context structure to the global list of context structures.
//
VOID CtxtListAddTo (PPER_SOCKET_CONTEXT lpPerSocketContext) {

	PPER_SOCKET_CONTEXT     pTemp;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("EnterCriticalSection raised an exception.\n");
        return;
    }

	if( g_pCtxtList == NULL ) {

		//
		// add the first node to the linked list
		//
		lpPerSocketContext->pCtxtBack    = NULL;
		lpPerSocketContext->pCtxtForward = NULL;
		g_pCtxtList = lpPerSocketContext;
	} else {

		//
		// add node to head of list
		//
		pTemp = g_pCtxtList;

		g_pCtxtList = lpPerSocketContext;
		lpPerSocketContext->pCtxtBack    = pTemp;
		lpPerSocketContext->pCtxtForward = NULL; 

		pTemp->pCtxtForward = lpPerSocketContext;
	}

	LeaveCriticalSection(&g_CriticalSection);
	return;
}

//
//  Remove a client context structure from the global list of context structures.
//
VOID CtxtListDeleteFrom(PPER_SOCKET_CONTEXT lpPerSocketContext) {

	PPER_SOCKET_CONTEXT pBack;
	PPER_SOCKET_CONTEXT pForward;
	PPER_IO_CONTEXT     pNextIO     = NULL;
	PPER_IO_CONTEXT     pTempIO     = NULL;

	
    __try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("EnterCriticalSection raised an exception.\n");
        return;
    }

	if( lpPerSocketContext ) {
		pBack       = lpPerSocketContext->pCtxtBack;
		pForward    = lpPerSocketContext->pCtxtForward;


		if( ( pBack == NULL ) && ( pForward == NULL ) ) {

			//
			// This is the only node in the list to delete
			//
			g_pCtxtList = NULL;
		} else if ( ( pBack == NULL ) && ( pForward != NULL ) ) {

			//
			// This is the start node in the list to delete
			//
			pForward->pCtxtBack = NULL;
			g_pCtxtList = pForward;
		} else if ( ( pBack != NULL ) && ( pForward == NULL ) ) {

			//
			// This is the end node in the list to delete
			//
			pBack->pCtxtForward = NULL;
		} else if( pBack && pForward ) {

			//
			// Neither start node nor end node in the list
			//
			pBack->pCtxtForward = pForward;
			pForward->pCtxtBack = pBack;
		}

		//
		// Free all i/o context structures per socket
		//
		pTempIO = (PPER_IO_CONTEXT)(lpPerSocketContext->pIOContext);
		do {
			pNextIO = (PPER_IO_CONTEXT)(pTempIO->pIOContextForward);
			if( pTempIO ) {

				//
				//The overlapped structure is safe to free when only the posted i/o has
				//completed. Here we only need to test those posted but not yet received 
				//by PQCS in the shutdown process.
				//
				if( g_bEndServer )
					while( !HasOverlappedIoCompleted((LPOVERLAPPED)pTempIO) ) Sleep(0);
				xfree(pTempIO);
				pTempIO = NULL;
			}
			pTempIO = pNextIO;
		} while( pNextIO );

		xfree(lpPerSocketContext);
		lpPerSocketContext = NULL;

	} else {
		myprintf("CtxtListDeleteFrom: lpPerSocketContext is NULL\n");
	}

	LeaveCriticalSection(&g_CriticalSection);
	return;
}

//
//  Free all context structure in the global list of context structures.
//
VOID CtxtListFree() {

	PPER_SOCKET_CONTEXT     pTemp1, pTemp2;

	__try
    {
        EnterCriticalSection(&g_CriticalSection);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        myprintf("EnterCriticalSection raised an exception.\n");
        return;
    }

	pTemp1 = g_pCtxtList; 
	while( pTemp1 ) {
		pTemp2 = pTemp1->pCtxtBack;
		CloseClient(pTemp1, FALSE);
		pTemp1 = pTemp2;
	}

	LeaveCriticalSection(&g_CriticalSection);
	return;
}

//
// Our own printf. This is done because calling printf from multiple
// threads can AV. The standard out for WriteConsole is buffered...
//
int myprintf (const char *lpFormat, ... ) {

	int nLen = 0;
	int nRet = 0;
	char cBuffer[512] ;
	va_list arglist ;
	HANDLE hOut = NULL;
    HRESULT hRet;

	ZeroMemory(cBuffer, sizeof(cBuffer));

	va_start(arglist, lpFormat);

	nLen = lstrlen( lpFormat ) ;
    hRet = StringCchVPrintf(cBuffer,512,lpFormat,arglist);
	
	if( nRet >= nLen || GetLastError() == 0 ) {
		hOut = GetStdHandle(STD_OUTPUT_HANDLE) ;
		if( hOut != INVALID_HANDLE_VALUE )
			WriteConsole( hOut, cBuffer, lstrlen(cBuffer), (LPDWORD)&nLen, NULL ) ;
	}

	return nLen ;
}
