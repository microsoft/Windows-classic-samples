// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      receiver.cpp
//
// Abstract: 
//      Implement receive-side functionality.  The reciever can accept up to 
//      MAXCLIENT connections from senders.  The receiver uses WSAEventSelect 
//      to handle the potential multiple connections as well as to allow the 
//      receiver handle a request for a QoS connection during the accept.
//
//

#include "atmevent.h"

#define MAXCLIENTS 4


// encapsulate socket, events, buffers, and various flags and counters that
// control how/when data is received into a structure
typedef struct _RECV_INFO
{
    SOCKET          sd[MAXCLIENTS+1];
    WSAEVENT        hEvents[MAXCLIENTS+1];
    CHAR *          recvbuf;
    int             nTotalClients;
} RECV_INFO;



static BOOL CreateReceivingSocket(RECV_INFO *, OPTIONS *);
static BOOL fd_accept_func(DWORD, WSANETWORKEVENTS *, RECV_INFO *, OPTIONS *);
static BOOL fd_read_func(DWORD, WSANETWORKEVENTS *, RECV_INFO *, OPTIONS *);
static BOOL fd_close_func(DWORD, WSANETWORKEVENTS *, RECV_INFO *, OPTIONS *);
int CALLBACK AcceptCondFunc(
                           LPWSABUF, LPWSABUF, LPQOS, LPQOS, LPWSABUF, LPWSABUF, GROUP FAR *, DWORD_PTR);




// Abstract:
//      Main function to create a socket that allows clients to connect and
//      send it data.  Upto MAXCLIENTS can connect and send data.  The data
//      is read in and then tossed (the data is not used).  
//
VOID Receiver(
             OPTIONS             *pOptions
             )
{
    WSANETWORKEVENTS NetworkEvents = {0};
    long lNetworkEvents = 0;
    BOOL bProcessEventsDone = FALSE;
    RECV_INFO RecvInfo = {0};
    int i = 0;

    printf("Receiver\n");

    //init RecvInfo sockets/event handles
    for (i = 0; i < MAXCLIENTS+1; i++)
    {
        RecvInfo.sd[i] = INVALID_SOCKET;
        RecvInfo.hEvents[i] = WSA_INVALID_EVENT;
    }


     

    for (i=0; i<MAXCLIENTS+1; i++)
    {
        if (WSA_INVALID_EVENT == (RecvInfo.hEvents[i] = WSACreateEvent()))
        {
            printf("WSACreateEvent(%d): %d\n", i, WSAGetLastError());
            goto CLEANUP;
        }
    }

    // create the receiving/listening socket
    if (!CreateReceivingSocket(&RecvInfo, pOptions))
        goto CLEANUP;


    lNetworkEvents = (FD_ACCEPT|FD_READ|FD_CLOSE);
    if (SOCKET_ERROR == WSAEventSelect(RecvInfo.sd[0], RecvInfo.hEvents[0], lNetworkEvents))
    {
        printf("WSAEventSelect: %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    bProcessEventsDone = FALSE;
    while (!bProcessEventsDone)
    {
        DWORD dwEvent;

        dwEvent = WSAWaitForMultipleEvents((MAXCLIENTS+1), RecvInfo.hEvents, FALSE, WSA_INFINITE, FALSE);
        switch (dwEvent)
        {
        case WSA_WAIT_FAILED:
            printf("WSAEventSelect: %d\n", WSAGetLastError());
            bProcessEventsDone = TRUE;
            break;
        case WAIT_IO_COMPLETION:
        case WSA_WAIT_TIMEOUT:
            break;

        default:
            // lets see what network activity trigged this event
            NetworkEvents.lNetworkEvents = 0;
            if (SOCKET_ERROR == WSAEnumNetworkEvents(RecvInfo.sd[dwEvent], RecvInfo.hEvents[dwEvent], &NetworkEvents))
            {
                printf("WSAEnumNetworkEvent: %d dwEvent %d lNetworkEvent %X\n", 
                       WSAGetLastError(), dwEvent, NetworkEvents.lNetworkEvents);
                NetworkEvents.lNetworkEvents = 0;
                bProcessEventsDone = TRUE;
            } else
            {
                // it is possible that more than one notification triggered the event
                //printf("WSAEnumNetworkEvents %X\n", NetworkEvents.lNetworkEvents);
                if (FD_ACCEPT & NetworkEvents.lNetworkEvents)
                    bProcessEventsDone |= 
                    fd_accept_func(dwEvent, &NetworkEvents, &RecvInfo, pOptions);
                if (FD_READ   & NetworkEvents.lNetworkEvents)
                    bProcessEventsDone |= 
                    fd_read_func(dwEvent, &NetworkEvents, &RecvInfo, pOptions);
                if (FD_CLOSE  & NetworkEvents.lNetworkEvents)
                    bProcessEventsDone |= 
                    fd_close_func(dwEvent, &NetworkEvents, &RecvInfo, pOptions);
            }
            break;
        }
    } // while (!bProcessEventsDone)


    CLEANUP:
    for (i = 0; i < MAXCLIENTS+1; i++)
    {
        if (INVALID_SOCKET != RecvInfo.sd[i])
        {
            closesocket(RecvInfo.sd[i]);
            RecvInfo.sd[i] = INVALID_SOCKET;
        }

        if (WSA_INVALID_EVENT != RecvInfo.hEvents[i])
        {
            WSACloseEvent(RecvInfo.hEvents[i]);
            RecvInfo.hEvents[i] = WSA_INVALID_EVENT;
        }
    }



    return;
}




// Abstract:
//      Create a socket on which to receive data.  Please note that WSA_FLAG_OVERLAPPED 
//      *MUST* be specified.  Also, note that the parameters for WSASocket and the 
//      data to fill in the SOCKADDR structure come from the supplied protocol 
//      info structure instead of being hardcoded.
//
static BOOL CreateReceivingSocket(
                                 RECV_INFO           *pRecvInfo, 
                                 OPTIONS             *pOptions
                                 )
{
    DWORD        dwSocketFlags = 0;
    SOCKADDR_ATM sockaddr = {0};
    int          nAddrLen = 0;
    int          nRet = 0;


    dwSocketFlags = WSA_FLAG_OVERLAPPED;

    pRecvInfo->sd[0] = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
                                 &pOptions->protocolInfo, 0, dwSocketFlags);             
    if (INVALID_SOCKET == pRecvInfo->sd[0])
    {
        printf("WSASocket failed w/%d", WSAGetLastError());
        return FALSE;
    }

    ZeroMemory(&sockaddr, sizeof(sockaddr));
    nAddrLen = sizeof(sockaddr);
    nRet = WSAStringToAddress(pOptions->szLocalInterface, AF_ATM, &pOptions->protocolInfo,
                              (LPSOCKADDR)&sockaddr, &nAddrLen);
    if (SOCKET_ERROR == nRet)
    {
        printf("WSAAddressToString: %d\n", WSAGetLastError());
        return FALSE;
    }
    // fill in remainder of ATM address structure
    sockaddr.satm_family                 = AF_ATM;
    sockaddr.satm_number.AddressType     = ATM_NSAP;
    sockaddr.satm_number.NumofDigits     = ATM_ADDR_SIZE;
    sockaddr.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
    sockaddr.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
    sockaddr.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;

    nRet = bind(pRecvInfo->sd[0], (LPSOCKADDR)&sockaddr, sizeof(sockaddr));
    if (SOCKET_ERROR == nRet)
    {
        printf("bind failed w/%d", WSAGetLastError());
        return FALSE;
    }

    nRet = listen(pRecvInfo->sd[0], SOMAXCONN);
    if (SOCKET_ERROR == nRet)
    {
        printf("listen failed w/%d", WSAGetLastError());
        return FALSE;
    }

    return TRUE;
}




// Abstract:
//      FD_ACCEPT handler.  Invoked when a client is trying to connect.
//      WSAAccept is called and an Accept Condition function is supplied to allow  
//      a connection to be rejected if too many client have already connected.
//      
//
static BOOL fd_accept_func(
                          DWORD               dwEvent, 
                          WSANETWORKEVENTS    *NetworkEvents, 
                          RECV_INFO           *pRecvInfo, 
                          OPTIONS             *pOptions
                          )
{
    BOOL    bProcessEventsDone = FALSE;
    SOCKET  temp_socket = INVALID_SOCKET;
    int     nRet = 0,
            i;


    printf("  FD_ACCEPT: dwEvent=%d error code =%d\n", 
           dwEvent, NetworkEvents->iErrorCode[FD_ACCEPT_BIT]);
    if (NetworkEvents->iErrorCode[FD_ACCEPT_BIT])
        return TRUE;

    temp_socket = WSAAccept(pRecvInfo->sd[0], NULL, NULL, (LPCONDITIONPROC)AcceptCondFunc, (DWORD_PTR)pOptions); 
    if (INVALID_SOCKET == temp_socket)
    {
        printf("    WSAAccept failed w/%d\n", WSAGetLastError());
        bProcessEventsDone = FALSE;
    } else
    {
        // keep list of connected sockets
        printf("    socket connection accepted\n");
        for (i=1; i<=MAXCLIENTS; i++)
        {
            if (INVALID_SOCKET == pRecvInfo->sd[i])
            {
                pRecvInfo->sd[i] = temp_socket;
                break;
            }
        }

        nRet = WSAEventSelect(temp_socket, pRecvInfo->hEvents[i], (FD_READ|FD_CLOSE));
        if (SOCKET_ERROR == nRet)
        {
            printf("    WSAEventSelect: failed w/%d\n", WSAGetLastError());
            bProcessEventsDone = TRUE;
        }
    }

    return bProcessEventsDone;
}



// Abstract:
//      FD_READ handler.  Handling FD_READ notifications is pretty straight-forward.  
//      Just read the amount of data you are interested in reading at this moment.  
//      If there is more data left, you will get another FD_READ.
//
//      
//
static BOOL fd_read_func(
                        DWORD               dwEvent, 
                        WSANETWORKEVENTS    *NetworkEvents, 
                        RECV_INFO           *pRecvInfo, 
                        OPTIONS             *pOptions
                        )
{
    BOOL        bProcessEventsDone = FALSE;
    int         nRecv = 0;
    static CHAR *bufptr = NULL;
    static int  nBytesToRecv = pOptions->nBufSize;


    __try
    {


        if (NULL == (pRecvInfo->recvbuf = (CHAR*)Malloc(pOptions->nBufSize)))
        {
            printf(" HeapAlloc() failed: %d [%s:%ld]\n",GetLastError(),__FILE__,__LINE__);
            __leave;
        }

        bufptr = pRecvInfo->recvbuf;

        printf("  FD_READ: dwEvent=%d error code =%d bytestorecv=%d\n", 
               dwEvent, NetworkEvents->iErrorCode[FD_READ_BIT], nBytesToRecv);
        if (NetworkEvents->iErrorCode[FD_READ_BIT])
        {
            bProcessEventsDone = TRUE;
            __leave;
        }
            

        nRecv = recv(pRecvInfo->sd[dwEvent], bufptr, nBytesToRecv, 0);
        if (SOCKET_ERROR == nRecv)
        {
            DWORD dwErr= WSAGetLastError();
            if (WSAEWOULDBLOCK != dwErr)
            {
                printf("    recv[from]: failed w/%d\n", WSAGetLastError());
                bProcessEventsDone = TRUE;
            } else
            {
                printf("    recv[from]: WSAEWOULDBLOCK, wait for next FD_READ\n");
                bProcessEventsDone = FALSE;
            }
        } else
        {
            // we got data
            if (nRecv)
                printf("    recv'ed bytes=%d char=<%c>\n", nRecv, *bufptr);
            else
                printf("    recv'ed 0 bytes\n"); 

            bufptr += nRecv;
            nBytesToRecv -= nRecv;
            if (0 == nBytesToRecv)
            {
                bufptr = pRecvInfo->recvbuf;
                nBytesToRecv = pOptions->nBufSize;
            }
        }

        
    }
    __finally
    {
        if(NULL != pRecvInfo->recvbuf) Free(pRecvInfo->recvbuf);
    }


    return bProcessEventsDone;
}





// Abstract:
//      FD_CLOSE handler.  When receiving FD_CLOSE, it is still possible for data 
//      to be queued up and therefore the socket should be drained of that data so
//      as not to lose that data.  Winsock2 events are manual reset events and FD_CLOSE 
//      has no re-enabling function, therefore must reset the event here, otherwise 
//      we would get additional FD_CLOSE notifications.
//
static BOOL fd_close_func(
                         DWORD               dwEvent, 
                         WSANETWORKEVENTS    *NetworkEvents, 
                         RECV_INFO           *pRecvInfo, 
                         OPTIONS             *pOptions
                         )
{
    BOOL bProcessEventsDone = FALSE;


    printf("  FD_CLOSE: dwEvent=%d error code =%d\n", 
           dwEvent, NetworkEvents->iErrorCode[FD_CLOSE_BIT]);
    if (NetworkEvents->iErrorCode[FD_CLOSE_BIT])
        bProcessEventsDone = TRUE;
    else
    {
        // no need to drain the socket if running of over native ATM
        WSAResetEvent(pRecvInfo->hEvents[dwEvent]);
        if (INVALID_SOCKET != pRecvInfo->sd[dwEvent])
        {
            closesocket(pRecvInfo->sd[dwEvent]);
            pRecvInfo->sd[dwEvent] = INVALID_SOCKET;
        }
        pOptions->dwTotalClients--;
        bProcessEventsDone = FALSE;
    }
    return bProcessEventsDone;
}




// Abstract:
//      Function called by Winsock to accept or reject a connection.  If more 
//      than MAXCLIENTS attempt to connect the connection will be rejected 
//      (CF_REJECT). 
//
//
int CALLBACK AcceptCondFunc(
                           LPWSABUF    lpCallerId,
                           LPWSABUF    lpCallerData,
                           LPQOS       lpSQos,
                           LPQOS       lpGQos,
                           LPWSABUF    lpCalleeId,
                           LPWSABUF    lpCalleeData,
                           GROUP FAR   *Group,
                           DWORD_PTR       CallbackData
                           )
{
    int     nRet        = CF_ACCEPT;
    OPTIONS *pOptions   = (OPTIONS *)CallbackData;

    lpCallerId;
    lpCallerData;
    lpCalleeId;
    lpCalleeData;
    lpSQos;
    lpGQos;
    Group;
    


    printf("    AcceptCondFunc\n");
    if (MAXCLIENTS == pOptions->dwTotalClients)
    {
        printf("      Rejecting connection, limit of %d clients reached\n",
               pOptions->dwTotalClients);
        nRet = CF_REJECT;
        // At this point a connection will already have been established,
        // this will just cause the connection to be reset.
    } else
        pOptions->dwTotalClients++;

    return nRet;
} 

