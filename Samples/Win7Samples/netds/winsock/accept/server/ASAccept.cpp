// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: ASAccept.cpp
//
// Description:
//             This file contains the functions for implementing the 
// Async Select version of the accept. Many of the functions that are common
// to Non-blocking accept and Async Select accept are defined in Common.cpp.

#include "common.h"

/*
    This function converts the FD_XXX events into a printable string.
    As the returned strings are static strings, they must not be modified.
*/
const char *FDImage(long lEvent)
{
    const char *szRetVal;

    // return the printable strings corresponding to the event.
    switch(lEvent)
    {
        case FD_READ:       szRetVal = "FD_READ";
                            break;
        case FD_WRITE:      szRetVal = "FD_WRITE";
                            break;
        case FD_ACCEPT:     szRetVal = "FD_ACCEPT";
                            break;
        case FD_CLOSE:      szRetVal = "FD_CLOSE";
                            break;
        default:            szRetVal = "Unexpected";
                            break;
    }

    return szRetVal;
}

/*
    This is the call back function for the WSAAsyncSelect call.
    This is called by winsock whenever a event is available on a
    AsyncSelect-ed socket to be processed.
    wParam contains the socket handle.
    lParam contains the signalled event as well as error code, if any.
*/
void ProcessAsyncSelectMessage(WPARAM wParam, LPARAM lParam)
{
    // get the socket handle from wParam.
    SOCKET sock = (SOCKET) wParam;

    // get the type of event using the macro below.
    long event = WSAGETSELECTEVENT(lParam);

    // get the error associated with the event, if any, using the macro below.
    int error = WSAGETSELECTERROR(lParam);
    BOOL bSocketToBeClosed = FALSE;
    PSOCK_INFO pSockInfo;    
    int rc;        

    printf("Entering ProcessAsyncSelectMessage() for "
           "Sock = %d, Event = %s, Error = %d\n",
            sock, FDImage(event), error);

    // first, get the SockInfo object corresponding to the signalled socket
    // to retrieve the context of this socket.
    for(pSockInfo = g_AcceptContext.pSockList;pSockInfo;
                                              pSockInfo = pSockInfo->next)        
    {
        if (sock == pSockInfo->sock)
            break; // found.
    }

    // check if we found one.
    if (!pSockInfo)
    {
        // no, we couldn't find one. This will happen usually because of
        // dummy FD_READs that we are posting which might show up even
        // after we closed the socket and removed the SockInfo structure
        // from the global sockets list. This condition is not an error,
        // but a normal timing condition. So, we don't have to do anything
        // with this socket.
        printf("Couldn't find SockInfo. Socket might have been closed.\n");
        goto CLEANUP;
    }

    // if we were signalled an error event instead of a positive event
    // then we don't have to process the socket.
    if (error)
    {
        // some error has happened on this socket.
        // need to close the socket.
        bSocketToBeClosed = TRUE;
        goto CLEANUP;
    }

    // take the corresponding action depending on the event signalled.    
    switch(event)
    {
        case FD_ACCEPT:
            
            // Accept the new connection.
            pSockInfo = ProcessAcceptEvent(pSockInfo);
            if (pSockInfo == NULL)
            {
                // some error in accepting the connection or allocating
                // resources for the new socket.
                break;
            }

            // request the specified events to be notified asynchronously
            // to the hAcceptWindow for the given sock.
            rc = WSAAsyncSelect(pSockInfo->sock, 
                                g_AcceptContext.hAcceptWindow,
                                WM_USER_ASYNCSELECT_MSG,
                                FD_READ | FD_WRITE | FD_CLOSE);

            // check if the request went through.            
            if (rc == SOCKET_ERROR)
            {
                printf("ERROR: WSAAsyncSelect failed on sock %d. Error = %d\n",
                       pSockInfo->sock, WSAGetLastError());
                bSocketToBeClosed = TRUE;
            }
            break;
                
        case FD_READ:

            // Read data from the socket, if there's room for the new data.
            bSocketToBeClosed = ProcessReadEvent(pSockInfo);

            // check if the recv call indicated that the remote side has closed
            // the socket and that we should also close it.
            if (bSocketToBeClosed == FALSE)
            {
                // no, it didn't. so, go ahead and echo the received data back
                // if possible.
                bSocketToBeClosed = SendData(pSockInfo);            

                // check if the echo was successful.
                if (bSocketToBeClosed == FALSE)
                {
                    // yes it was. 
                    // now, if this happens to be the last piece of the
                    // incoming data, and a FD_CLOSE was already notified to
                    // us, we may not receive any more FD_READ notifications.
                    // So, we need to post a dummy FD_READ notification to
                    // this socket so that we again do a recv until we get
                    // a 0 from recv, which means graceful disconnect from
                    // other side or until we receive an error from recv.
                    if (pSockInfo->isFdCloseRecd)
                    {
                        // post ourselves another FD_READ in case 
                        // there's no data left.
                        printf("Posting a dummy FD_READ \n");
                        PostMessage(g_AcceptContext.hAcceptWindow,
                                    WM_USER_ASYNCSELECT_MSG,
                                    pSockInfo->sock,
                                    FD_READ);
                    }
                }
            }
            break;
            
        case FD_WRITE:

            // since we usually echo the data immediately after receiving it
            // in the FD_READ case above, we may not need this case, except
            // for one important condition: when send fails with WSAEWOULDBLOCK
            // then we'll be notified with this event once the remote side
            // has started consuming the data and we can continue our send.
            bSocketToBeClosed = SendData(pSockInfo);

            // in this case, most likely, while the previous data wasn't
            // sent, there would have been a FD_READ which didn't result
            // in a recv call. Hence, there won't be another FD_READ posted
            // unless we call recv. So, we post a dummy FD_READ ourselves
            // to allow the FD_READ handling code to be explicitly called.

            // Note: In a real implementation, we would have to maintain
            // more elaborate state to avoid posting these possibly redundant
            // window messages.
            
            printf("Posting a dummy FD_READ \n");
            PostMessage(g_AcceptContext.hAcceptWindow,
                        WM_USER_ASYNCSELECT_MSG,
                        pSockInfo->sock,
                        FD_READ);
            
            break;
            
        case FD_CLOSE:

            // even though the remote side has closed the socket, there might
            // still be data that's left to read. so, we should just remember
            // that we received a FD_CLOSE but not close the socket until
            // we have read all the data.
            pSockInfo->isFdCloseRecd = TRUE;

            // post ourselves another FD_READ in case there's no data
            // left in which case, we'll actually close the socket when recv
            // returns 0.
            printf("Posting a dummy FD_READ \n");
            PostMessage(g_AcceptContext.hAcceptWindow,
                        WM_USER_ASYNCSELECT_MSG,
                        pSockInfo->sock,
                        FD_READ);
            
            break;
            
        default:
            printf("Unexpected event: 0x%x for socket %d\n", event, sock);
            bSocketToBeClosed = TRUE;
            break;
    }

CLEANUP:

    // if we were told in any of the above cases to close the socket,
    // e.g. remote side closed and we got an error or recv returned 0,etc.
    // then we close our socket.
    if (bSocketToBeClosed)
    {
        // make sure we cancel the WSAAsyncSelect event with the event mask 0.
        WSAAsyncSelect(pSockInfo->sock, g_AcceptContext.hAcceptWindow, 0, 0);

        // shutdown the socket so as not to hang the client just in case it
        // expects us to send any more data.
        if (shutdown(pSockInfo->sock, SD_SEND) == SOCKET_ERROR)
            printf("ERROR: Shutdown failed. Error = %d\n", WSAGetLastError());

        // close the socket.
        closesocket(pSockInfo->sock);
        printf("Closed socket %d. "
               "Total Bytes Recd = %d, "
               "Total Bytes Sent = %d\n",
                pSockInfo->sock, 
                pSockInfo->nTotalRecd,
                pSockInfo->nTotalSent);

        // delete it from the global list and free the memory.
        DeleteSockInfoFromList(&g_AcceptContext.pSockList,
                               pSockInfo);
    }
    
    printf("Exiting ProcessAsyncSelectMessage()\n");
	return ;
}


/*
    This is the Windows procedure for handling all the messages for
    the given window. We are interested in only one message. The
    rest of them will be passed on to the default windows procedure.
*/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam)
{
    LRESULT res = 0;

    // check what the message type is.
    switch (message) {

        case WM_DESTROY:
    		PostQuitMessage(0);
            break;

    	// this is the message that that we gave in WSAAsyncSelect which
    	// winsock will send us back for notifying us of socket events.
        case WM_USER_ASYNCSELECT_MSG:
       		ProcessAsyncSelectMessage(wParam, lParam);
    		break;

    	default: 
    		res = DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }

	return res;
}


/*
    This function creates a hidden window to receive the 
    async select messages sent by winsock for socket events.
*/
HWND CreateAcceptWindow(void)
{
	WNDCLASS window;
	HWND windowHandle = NULL;

    // set the window properties.
	memset(&window, 0, sizeof(WNDCLASS));
	window.lpszClassName = "Accept Window";
	window.hInstance = NULL;
	window.lpfnWndProc = (WNDPROC) MainWndProc;	
	window.hCursor = NULL;	
	window.hIcon = NULL;
	window.lpszMenuName = NULL;
	window.hbrBackground = NULL; 
	window.style = 0;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;

	// register the window class.
	if (!RegisterClass(&window)) {
		printf("Registerclass failed %d\n", GetLastError());
        goto CLEANUP;
	}

	// create the window. 
	windowHandle = CreateWindow("Accept Window",
								"Accept Window",
								WS_OVERLAPPEDWINDOW,	//WS_MINIMIZE,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								(HWND) NULL,
								(HMENU) NULL,
								(HINSTANCE) NULL,
								(LPVOID) NULL);

	// check if a window was created.
	if (windowHandle == NULL) 
	{
		printf("CreateWindow failed %d\n", GetLastError());
		return NULL;
	}

    // uncomment the line below to see the window on the taskbar.
    // ShowWindow(windowHandle, SW_MINIMIZE);
	
CLEANUP:
	
	return windowHandle;
}



/*
    This function is the entry point for the AsyncSelect Accept implementation.
*/
void AsyncSelectAcceptMain()
{
    PSOCK_INFO pSockInfo;
    int rc;
    MSG msg;
     
    printf("Entering AsyncSelectAcceptMain()\n");

    // create a dummy hidden window to receive the async select messages.
    g_AcceptContext.hAcceptWindow = CreateAcceptWindow();
    if (g_AcceptContext.hAcceptWindow == NULL)
    {
        printf("Error in creating the accept window.\n");
        goto CLEANUP;
    }

    // Set for all the listening sockets to be signalled on FD_ACCEPT event.
    for(pSockInfo = g_AcceptContext.pSockList; pSockInfo != NULL;
                                               pSockInfo = pSockInfo->next)
    {
        // request asynchronous notifications for the FD_ACCEPT and FD_CLOSE
        // events to be sent to the hAcceptWindow for this sock.
        rc = WSAAsyncSelect(pSockInfo->sock,
                            g_AcceptContext.hAcceptWindow, 
                            WM_USER_ASYNCSELECT_MSG,
                            FD_ACCEPT | FD_CLOSE);
        if (rc == SOCKET_ERROR)
        {
            printf("ERROR: WSAAsyncSelect failed for sock %d. Error = %d\n", 
                                    pSockInfo->sock, WSAGetLastError());
            continue;                                                  
        }
    }

    // Main message loop to process messages to the Accept Window.
    // This will indirectly call MainWndProc which will in turn call
    // ProcessAsyncSelectMessage.
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 

   
CLEANUP:        

    printf("Exiting AsyncSelectAcceptMain()\n");
    return ;
}
