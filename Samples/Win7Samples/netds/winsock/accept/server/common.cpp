// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: Common.cpp
//
// Description:
//             This file contains the functions that are common to
// all types of accept. This includes creating and destroying the listening
// sockets on the requested interfaces and actual processing of the accepted
// socket. These are common because non-blocking accept and AsyncSelect accept
// only differ in the way they are notified to the app, not in the way
// the accepted socket is processed.

#pragma warning (disable: 4267)

#include "common.h"


/*
    Prints the given socket address in a printable string format.
*/
void PrintAddressString(LPSOCKADDR pSockAddr, DWORD dwSockAddrLen)
{
    // INET6_ADDRSTRLEN is the maximum size of a valid IPv6 address 
    // including port,colons,NULL,etc.
    char buf[INET6_ADDRSTRLEN];
    DWORD dwBufSize = 0;    

    memset(buf,0,sizeof(buf));
    dwBufSize = sizeof(buf);

    // This function converts the pSockAddr to a printable format into buf.   
    if (WSAAddressToString(pSockAddr, 
                           dwSockAddrLen, 
                           NULL, 
                           buf, 
                           &dwBufSize) == SOCKET_ERROR)
    {
        printf("ERROR: WSAAddressToString failed %d \n", WSAGetLastError());
        goto CLEANUP;
    }

    printf("%s\n", buf);

CLEANUP:
    return;
}



/*
    This function creates a list of sockets for each of the interfaces
    on which the server is supposed to listen. For each socket, it binds
    it to the requested port and sets up for listening on the socket.
*/
void CreateListeningSockets()
{
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *pAddr;
    SOCKET newSock;
    PSOCK_INFO pNewSockInfo;
    int i;
    unsigned long nonBlocking = 1;

    printf("Entering CreateListeningSockets()\n");

    printf("Creating the list of sockets to listen for ...\n");

    // prepare the hints for the type of socket we are interested in.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = g_AcceptContext.addressFamily;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // since we're going to bind on this socket.

    // getaddrinfo is the protocol independent version of GetHostByName.
    // the res contains the result.
    if (getaddrinfo(g_AcceptContext.szInterface, 
                g_AcceptContext.szPort,
                &hints, 
                &res) != NO_ERROR)
    {
        printf("getaddrinfo failed. Error = %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    if (res == NULL)
    {
        printf("getaddrinfo returned res = NULL\n");
        goto CLEANUP;
    } 

    printf("getaddrinfo successful.Enumerating the returned addresses ...\n\n");

    // for each returned interface, create a listening socket.
    for (pAddr = res, i = 1; pAddr != NULL; pAddr = pAddr->ai_next, i++)
    {
        printf("Processing Address %d returned by getaddrinfo : ", i);
        PrintAddressString(pAddr->ai_addr, pAddr->ai_addrlen);

        // create a suitable socket for this interface.
        newSock = WSASocket(pAddr->ai_family, 
                            pAddr->ai_socktype,
                            pAddr->ai_protocol,
                            NULL,
                            NULL,
                            0);
        if (newSock == INVALID_SOCKET)
        {
            printf("WSASocket failed. Error = %d\n", WSAGetLastError());
            printf("Ignoring this address and continuing with the next. \n\n");
            
            // anyway, let's continue with other addresses.
            continue;
        }

        printf("Created socket with handle = %d\n", newSock);

        // bind the socket.
        if (bind(newSock, pAddr->ai_addr, pAddr->ai_addrlen) != NO_ERROR)
        {
            printf("bind failed. Error = %d\n", WSAGetLastError());
            closesocket(newSock);
            continue;
        }

        printf("Socket bound successfully\n");        

        // listen for upto MAX_CLIENTS number of clients.
        if (listen(newSock, MAX_CLIENTS) != NO_ERROR)
        {
            printf("listen failed. Error = %d\n", WSAGetLastError());
            closesocket(newSock);
            continue;
        }

        printf("Listen successful\n");

        // for non-blocking select, we need to explicitly make the socket
        // non-blocking by this call, whereas for WSAAsyncSelect this is
        // not required as WSAAsyncSelect itself makes the socket non-blocking.
        if (g_AcceptContext.typeOfAccept == NON_BLOCKING_ACCEPT)
        {
            if (ioctlsocket(newSock, FIONBIO, &nonBlocking) == SOCKET_ERROR)
            {
                printf("Can't put socket into non-blocking mode. Error = %d\n",
                        WSAGetLastError());
            }
        }

        // allocate a socket info and store this socket.
        pNewSockInfo = AllocAndInitSockInfo();
        if (pNewSockInfo == NULL)
        {
            printf("AllocAndInitSockInfo failed.\n");
            closesocket(newSock);
            continue; 
        }

        pNewSockInfo->sock = newSock;
        pNewSockInfo->isSocketListening = TRUE;  

        // all went well. add this to the list of listening sockets.
        AddSockInfoToList(&g_AcceptContext.pSockList, pNewSockInfo);
        printf("Added socket to list of listening sockets\n\n");
    }

   
CLEANUP:

    // if getaddrinfo was successful, it would have allocated memory for the
    // res pointer which needs to be freed.
    if (res)
    {
        freeaddrinfo(res);
        printf("Freed the memory allocated for res by getaddrinfo\n");
    }

    printf("Exiting CreateListeningSockets()\n");
    return;
}



/*
    This function frees all the resources inside the socket list.
*/
void DestroyListeningSockets()
{
    PSOCK_INFO pSockInfo,
               pNextSockInfo;

    printf("Entering DestroyListeningSockets()\n");

    // iterate through all the listening sockets and free one by one.
    pSockInfo = g_AcceptContext.pSockList; 
    while (pSockInfo != NULL)
    {
        // if the socket hasn't been closed already, close it.
        if (pSockInfo->sock != INVALID_SOCKET)
        {
            closesocket(pSockInfo->sock);
            printf("Closed socket with handle %d\n", pSockInfo->sock);
        }

        // free this socket and go to the next.
        pNextSockInfo = pSockInfo->next;
        FreeSockInfo(pSockInfo);
        pSockInfo = pNextSockInfo;
    }

    printf("Exiting DestroyListeningSockets()\n");
    return ;
}


/*
    This function does a accept on the socket in which an accept event has
    been signalled. It also adds the accepted socket to the global list of
    sockets.
*/
PSOCK_INFO ProcessAcceptEvent(PSOCK_INFO pSockInfo)
{
    SOCKADDR_STORAGE clientAddress;
    int clientAddressLen = sizeof(clientAddress);
    SOCKET newSock;
    PSOCK_INFO pNewSockInfo = NULL;
    unsigned long nonBlocking = 1;

    printf("Entering ProcessAcceptEvent() on socket %d\n", pSockInfo->sock);

    // accept the new connection.
    newSock = WSAAccept(pSockInfo->sock, 
                  (LPSOCKADDR)&clientAddress, 
                  &clientAddressLen, 
                  NULL, 
                  NULL);
    if (newSock == INVALID_SOCKET)
    {
        printf("ERROR: WSAAccept failed. Error = %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    printf("Accepted connection from client:");
    PrintAddressString((LPSOCKADDR) &clientAddress, clientAddressLen);

    // set the connected socket to non-blocking mode.
    if (ioctlsocket(pSockInfo->sock, FIONBIO, &nonBlocking) == SOCKET_ERROR)
    {
        printf("Can't put socket into non-blocking mode. Error = %d\n",
                WSAGetLastError());
    }

    // add this socket to the global sockets list.             
    pNewSockInfo = AllocAndInitSockInfo();
    if (pNewSockInfo == NULL)
    {
        printf("AllocAndInitSockInfo failed.\n");
        closesocket(newSock);
        goto CLEANUP;
    }

    pNewSockInfo->sock = newSock;
    pNewSockInfo->isSocketListening = FALSE;    
    AddSockInfoToList(&g_AcceptContext.pSockList, pNewSockInfo);
    printf("Added accepted socket %d to list of sockets\n", newSock);    
                   
CLEANUP:
    printf("Exiting ProcessAcceptEvent()\n"); 
    return pNewSockInfo;
}


/*
      This functions reads incoming data on the given socket in which a
      read event has been signalled. It read new data only if the previously
      received data has been fully sent. 
*/
BOOL ProcessReadEvent(PSOCK_INFO pSockInfo)
{
    BOOL bSocketError = FALSE;   
    int nBytesRecd;    
    int err;
    
    printf("Entering ProcessReadEvent() on socket %d\n", pSockInfo->sock); 

    // check if the previously received data has been fully sent or not.
    if (pSockInfo->recdData.isNewData)
    {
        printf("Previously recd data not yet fully sent.\n");
        // here, since we are using only a fixed buffer size for outstanding
        // sends, we're not reading the data and hence there might be a
        // deadlock if the remote side only sends but doesn't receive at all.
        // also, if the remote side has closed without reading all the
        // data in the pipe, we may not know about it as we might get the
        // FD_CLOSE event only after all the FD_READ events have been
        // processed. so, to avoid these, the app should implement a timeout
        // on each socket so that malicious clients don't make the server
        // run out of resources and do a denial-of-service attack.

        // on the other hand, if we dynamically allocated memory for each
        // recv and queued it, we may not hit this deadlock, but still such
        // malicious clients can send too many data and not read at all and
        // thus perform a denial-of-service attack by making server run out
        // of memory. so, there has to be a upper limit for per-socket 
        // memory usage or timeout to avoid such attacks.
        Sleep(2000);
        goto CLEANUP;
    }

    // previous data fully sent, now, recv the new data.
    memset(&pSockInfo->recdData,0,sizeof(pSockInfo->recdData));
    
    nBytesRecd = recv(pSockInfo->sock, 
                      pSockInfo->recdData.buf, 
                      RECV_DATA_SIZE - 1, 
                      0);
    if (nBytesRecd < 0)
    {
        err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
            printf("recv got WSAEWOULDBLOCK. Will retry recv later ...\n");
            bSocketError = FALSE; // not a real error.
        }
        else
        {
            printf("ERROR: recv failed. error = %d\n", err);
            bSocketError = TRUE;
        }
        goto CLEANUP;        
    }
    if (nBytesRecd == 0)
    {
        printf("recv returned 0. Remote side has closed gracefully. Good.\n");
        bSocketError = TRUE;        
        goto CLEANUP;
    }

    // update the stats.
    pSockInfo->recdData.dataSize = nBytesRecd;
    pSockInfo->nTotalRecd += nBytesRecd;
    pSockInfo->recdData.isNewData = TRUE;    
    
    printf("Received data = %s, length = %d\n", pSockInfo->recdData.buf, 
                                                nBytesRecd);
    
CLEANUP:

    printf("Exiting ProcessReadEvent()\n"); 
    return bSocketError;

}

/*
    This functions sends data on the socket in which a write event has been
    signalled. If sends only if the socket has some newly received data to
    send. In case if the entire data in the recdData buffer could not be
    sent, this function remembers the offset at which to resume the next
    send. It signals that the data has been "fully" sent only after sending
    all of the data in the buffer.
*/
BOOL SendData(PSOCK_INFO pSockInfo)
{
    BOOL bSocketError = FALSE;
    int nBytesSent;
    int err;
    
    printf("Entering SendData() on socket %d\n", pSockInfo->sock);

    // check if there's any data received in the buffer after the last one
    // we sent out.
    if (!pSockInfo->recdData.isNewData || pSockInfo->recdData.dataSize <= 0)
    {
        printf("No data pending to be sent.\n");
        goto CLEANUP;
    }

    // if the data buffer contains new data, begin sending from the start
    // otherwise, begin sending from the point we left in the last call.
    nBytesSent = send(pSockInfo->sock, 
                      pSockInfo->recdData.buf + pSockInfo->recdData.sendOffset,
                      pSockInfo->recdData.dataSize, 
                      0);
    if (nBytesSent == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
            printf("send got WSAEWOULDBLOCK. Will retry send later ...\n");
            bSocketError = FALSE; // not a real error.
        }
        else
        {
            printf("ERROR: send failed. error = %d\n", err);
            bSocketError = TRUE;
        }
        goto CLEANUP;
    }

    // update the stats.
    pSockInfo->recdData.sendOffset += nBytesSent;
    pSockInfo->recdData.dataSize -= nBytesSent;

    // consider data as "fully" sent only when the entire buf has been sent.
    if (pSockInfo->recdData.dataSize == 0)
        pSockInfo->recdData.isNewData = FALSE;
    pSockInfo->nTotalSent += nBytesSent;
    printf("Sent %d bytes. Remaining = %d bytes.\n", nBytesSent, 
                                        pSockInfo->recdData.dataSize);    

CLEANUP:

    printf("Exiting SendData()\n");
    return bSocketError;
}



