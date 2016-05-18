// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      sender.cpp
//
// Abstract:
//      Implement send-side functionality.  The sender uses a combination of
//      WSASend and WSAEventSelect to support sending of data on the socket. 
//
// Entry Points:
//      Sender - send contents of data buffer to receiver program.
//

#include "atmevent.h"


#define WSAEVENTSELECT_EVENT 0
#define WSASEND_EVENT 1
#define TOTAL_EVENTS 2


// encapsulate socket, events, buffers, and various flags and counters that
// control how/when data is sent into a structure
typedef struct _SEND_INFO
{
    SOCKET          sd;             
    WSAEVENT        hEvents[2];     // WSASend and WSAEventSelect events
    WSAOVERLAPPED   ovr;
    BOOL            bSendEnabled;
    BOOL            bSendComplete;
    int             nTotalSends;
    WSABUF          sndbuf;
} SEND_INFO;



static BOOL CreateSendingSocket(SEND_INFO *, OPTIONS *);
static BOOL fd_connect_func(DWORD, WSANETWORKEVENTS *, SEND_INFO *, OPTIONS *);
static BOOL fd_close_func(DWORD, WSANETWORKEVENTS *, SEND_INFO *, OPTIONS *);
static BOOL PostSend(SEND_INFO *, OPTIONS *);
static BOOL CompleteSend(SEND_INFO *, OPTIONS *);



// Abstract:
//      Main function to establish connection to a server and then 
//      send data to server.  WSASend and WSAEventSelect are used to send data 
//      and monitor socket activity, respectively.  
//      
//      Due to the nature of this application - just sending data in a loop - 
//      FD_WRITE is not suitable whereas using WSASend with overlapped I/O is.  
//      The reason is because FD_WRITE will only be delivered after the inital 
//      connection has been established and only after sends block would 
//      WSAEWOULDBLOCK and buffer space becomes available once again.  Thus 
//      WSAWaitForMultipleEvents waits on the event associated with overlapped 
//      structure passed to WSASend AND the event associated with WSAEventSelect.
//
VOID Sender(
           OPTIONS             *pOptions
           )
{
    WSANETWORKEVENTS    NetworkEvents = {0};
    BOOL                bProcessEventsDone = FALSE;
    SEND_INFO           SendInfo = {0};
    int                 i = 0;
    
    printf("Sender\n");

    SendInfo.sd = INVALID_SOCKET;

    for (i = 0; i < TOTAL_EVENTS; i++)
        SendInfo.hEvents[i] = WSA_INVALID_EVENT;
    


    for (i=0; i<TOTAL_EVENTS; i++)
    {
        if (WSA_INVALID_EVENT == (SendInfo.hEvents[i] = WSACreateEvent()))
        {
            printf("WSACreateEvent(%d): failed w/%d\n", i, WSAGetLastError());
            goto CLEANUP;
        }
    }
    SendInfo.sd           = INVALID_SOCKET;
    ZeroMemory((char *)&SendInfo.ovr, sizeof(SendInfo.ovr));
    SendInfo.ovr.hEvent   = SendInfo.hEvents[WSASEND_EVENT];
    SendInfo.sndbuf.buf   = pOptions->buf;
    SendInfo.sndbuf.len   = pOptions->nBufSize;
    SendInfo.nTotalSends  = 0;
    SendInfo.bSendEnabled = TRUE;    

    if (!CreateSendingSocket(&SendInfo, pOptions))
        goto CLEANUP;


    while (!bProcessEventsDone)
    {
        DWORD dwEvent;

        dwEvent = WSAWaitForMultipleEvents(TOTAL_EVENTS, SendInfo.hEvents, FALSE, WSA_INFINITE, FALSE);
        switch (dwEvent)
        {
        case WSA_WAIT_FAILED:
            printf("WSAEventSelect: %d\n", WSAGetLastError());
            break;
        case WAIT_IO_COMPLETION:
        case WSA_WAIT_TIMEOUT:
            break;

        default:
            dwEvent -= WSA_WAIT_EVENT_0;

            if (WSAEVENTSELECT_EVENT == dwEvent)
            {
                // lets see what network activity trigged this event
                if (SOCKET_ERROR == WSAEnumNetworkEvents(SendInfo.sd, SendInfo.hEvents[WSAEVENTSELECT_EVENT], &NetworkEvents))
                {
                    printf("WSAEnumNetworkEvent: failed w/%d lNetworkEvent %X\n", 
                           WSAGetLastError(), NetworkEvents.lNetworkEvents);
                    NetworkEvents.lNetworkEvents = 0;
                } else
                {
                    if (FD_CONNECT     & NetworkEvents.lNetworkEvents)
                        bProcessEventsDone |= 
                        fd_connect_func(dwEvent, &NetworkEvents, &SendInfo, pOptions);
                    if (FD_CLOSE   & NetworkEvents.lNetworkEvents)
                        bProcessEventsDone |= 
                        fd_close_func(dwEvent, &NetworkEvents, &SendInfo, pOptions);
                }
            } else
            {
                bProcessEventsDone = CompleteSend(&SendInfo, pOptions);
            }
        }

        // allow pauses between notifications
        if (pOptions->dwSleep)
            Sleep(pOptions->dwSleep);

    } // while (!bProcessEventsDone)


    CLEANUP:
    if (INVALID_SOCKET != SendInfo.sd)
    {
        closesocket(SendInfo.sd);
        SendInfo.sd = INVALID_SOCKET;
    }
    for (i=0; i<TOTAL_EVENTS; i++)
        if(WSA_INVALID_EVENT != SendInfo.hEvents[i])
        {
            WSACloseEvent(SendInfo.hEvents[i]);
            SendInfo.hEvents[i] = WSA_INVALID_EVENT;
        }
    
        

    return;
}




// Abstract:
//      Create a socket on which to send data.  Please note that WSA_FLAG_OVERLAPPED 
//      *MUST* be specified.  Also, note that the parameters for WSASocket and the data 
//      to fill in the SOCKADDR structure come from the supplied protocol info structure 
//      instead of being hardcoded.
//
//      The socket can request the connection to support a specific QoS during the connect.
//
static BOOL CreateSendingSocket(
                               SEND_INFO           *pSendInfo, 
                               OPTIONS             *pOptions
                               )
{
    DWORD           dwSocketFlags = 0;
    SOCKADDR_ATM    sockaddr = {0};
    QOS             *pQos = NULL;
    int             nAddrLen = 0;
    long            lNetworkEvents = 0;
    int             nRet = 0;


    dwSocketFlags = WSA_FLAG_OVERLAPPED;
    pSendInfo->sd = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
                              &pOptions->protocolInfo, 0, dwSocketFlags);             
    if (INVALID_SOCKET == pSendInfo->sd)
    {
        printf("socket failed w/%d", WSAGetLastError());
        return FALSE;
    }

    // set up the address structure of the destination which is needed on connect 
    // and/or sendto operations.
    ZeroMemory(&sockaddr, sizeof(sockaddr));
    nAddrLen = sizeof(sockaddr);
    nRet = WSAStringToAddress(pOptions->szRemoteInterface, AF_ATM, &pOptions->protocolInfo,
                              (LPSOCKADDR)&sockaddr, &nAddrLen);
    if (SOCKET_ERROR == nRet)
    {
        printf("WSAAddressToString: failed w/%d\n", WSAGetLastError());
        return FALSE;
    }
    // fill in remainder of ATM address structure not set using WSAStringToAddress
    sockaddr.satm_family                 = AF_ATM;
    sockaddr.satm_number.AddressType     = ATM_NSAP;
    sockaddr.satm_number.NumofDigits     = ATM_ADDR_SIZE;
    sockaddr.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
    sockaddr.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
    sockaddr.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;


    lNetworkEvents = (FD_CONNECT | FD_CLOSE);
    if (SOCKET_ERROR == WSAEventSelect(pSendInfo->sd, pSendInfo->hEvents[WSAEVENTSELECT_EVENT], lNetworkEvents))
    {
        printf("WSAEventSelect: failed w/%d\n", WSAGetLastError());
        return FALSE;
    }

    pQos = NULL;
    nRet = WSAConnect(pSendInfo->sd, 
                      (LPSOCKADDR)&sockaddr, sizeof(sockaddr), 
                      NULL, NULL, pQos, NULL);
    if (SOCKET_ERROR == nRet)
    {
        DWORD dwErr = WSAGetLastError();
        if (WSAEWOULDBLOCK != dwErr)
        {
            printf("WSAConnect: failed w/%d\n", dwErr);
            return FALSE;
        }
    }

    return TRUE;
}







// Abstract:
//      FD_CONNECT handler.  This handler is invoked when an asynchronous connect
//      completes, successfully or otherwise.  After the socket connects 
//      additional notifications will be enabled using WSAEventSelect.
//
//
static BOOL fd_connect_func(
                           DWORD               dwEvent, 
                           WSANETWORKEVENTS    *NetworkEvents, 
                           SEND_INFO           *pSendInfo, 
                           OPTIONS             *pOptions
                           )
{
    BOOL    bProcessEventsDone = FALSE;
    int     nRet = 0;


    printf("  FD_CONNECT: dwEvent=%d error code =%d\n", 
           dwEvent, NetworkEvents->iErrorCode[FD_CONNECT_BIT]);
    if (NetworkEvents->iErrorCode[FD_CONNECT_BIT] != 0)
        bProcessEventsDone = TRUE;
    else
    {
        nRet = WSAEventSelect(pSendInfo->sd, pSendInfo->hEvents[WSAEVENTSELECT_EVENT], (FD_CLOSE));
        if (SOCKET_ERROR == nRet)
        {
            printf("WSAEventSelect: failed w/%d\n", WSAGetLastError());
            bProcessEventsDone = FALSE;
        }

        pSendInfo->bSendEnabled = TRUE;

        // post initial send 
        bProcessEventsDone = PostSend(pSendInfo, pOptions);
    }

    return bProcessEventsDone;
}





// Abstract:
//      FD_CLOSE handler.   If server closes before we have sent all the data, then 
//      we are done.  Winsock2 events are manual reset events and FD_CLOSE has no 
//      re-enabling function, therefore must reset the event here, otherwise we would 
//      get additional FD_CLOSE notifications.
//
static BOOL fd_close_func(
                         DWORD               dwEvent, 
                         WSANETWORKEVENTS    *NetworkEvents, 
                         SEND_INFO           *pSendInfo, 
                         OPTIONS             *pOptions
                         )
{
    BOOL bProcessEventsDone = TRUE;

    printf("  FD_CLOSE: dwEvent=%d error code =%d\n", 
           dwEvent, NetworkEvents->iErrorCode[FD_CLOSE_BIT]);
    WSAResetEvent(pSendInfo->hEvents[WSAEVENTSELECT_EVENT]);
    if (pOptions->dwSleepClose)
        Sleep(pOptions->dwSleepClose);
    if (INVALID_SOCKET != pSendInfo->sd)
    {
        closesocket(pSendInfo->sd);
        pSendInfo->sd = INVALID_SOCKET;
    }
    return bProcessEventsDone;
}




// Abstract:
//      Post an overlapped send and indicate a send is in progress if it doesn't
//      complete immediately.  If the WSASend completes immediately bump the
//      counters and indicate the send completed.  The send flag is needed to  
//      what needs to be done in CompleteSend() - wait for overlapped results
//      or post another send.
//      
static BOOL PostSend(
                    SEND_INFO   *pSendInfo, 
                    OPTIONS     *pOptions
                    )
{
    int     nRet = 0;
    DWORD   cbBytesXfer = 0;
    BOOL    bProcessEventsDone = FALSE;

    pOptions = NULL;


    printf("  PostSend\n");
    if (pSendInfo->bSendEnabled)
    {
        pSendInfo->bSendComplete = FALSE;
        nRet = WSASend(pSendInfo->sd, &pSendInfo->sndbuf, 1, &cbBytesXfer, 0, 
                       &pSendInfo->ovr, NULL);
        if (0 == nRet)
        {
            // the send completed immediately
            pSendInfo->bSendComplete = TRUE;
            pSendInfo->sndbuf.len -= cbBytesXfer;
            pSendInfo->sndbuf.buf += cbBytesXfer;
        } else if (SOCKET_ERROR == nRet)
        {
            DWORD dwErr = WSAGetLastError();
            if (WSA_IO_PENDING != dwErr)
            {
                printf("WSASend failed w/%d\n", dwErr);
                bProcessEventsDone = TRUE;
            }
        }
    }
    return bProcessEventsDone;
}




// Abstract:
//      Handle the completion of a WSASend.  This may involve posting another 
//      overlapped send or closing the socket. 
//
static BOOL CompleteSend(
                        SEND_INFO   *pSendInfo, 
                        OPTIONS     *pOptions
                        )
{
    BOOL    bProcessEventsDone = FALSE;
    DWORD   dwFlags = 0;
    DWORD   cbBytesXfer = 0;

    printf("  CompleteSend\n");
    if (!pSendInfo->bSendComplete)
    {
        // handle getting overlapped results
        if (!WSAGetOverlappedResult(pSendInfo->sd, &pSendInfo->ovr, &cbBytesXfer, TRUE, &dwFlags))
        {
            printf("WSAGetOverlappedResult = %d\n", WSAGetLastError());
            bProcessEventsDone = TRUE;
        } else
        {
            pSendInfo->bSendComplete = TRUE;
            pSendInfo->sndbuf.len -= cbBytesXfer;
            pSendInfo->sndbuf.buf += cbBytesXfer;
        }
    }

    if (pSendInfo->sndbuf.len > 0)
    {
        PostSend(pSendInfo, pOptions);
        bProcessEventsDone = FALSE;
    } else
    {
        // we have sent the entire buffer, check repeat count to
        // see if buffer needs to be sent again.
        if (pOptions->nRepeat)
            pSendInfo->nTotalSends++;
        else
            pSendInfo->nTotalSends = -1;
        pSendInfo->sndbuf.buf = pOptions->buf;
        pSendInfo->sndbuf.len = pOptions->nBufSize;
        if (pSendInfo->nTotalSends <= pOptions->nRepeat-1)
        {
            PostSend(pSendInfo, pOptions);
            bProcessEventsDone = FALSE;
        } else
        {
            printf("    repeat count complete - closing\n");
            if (pOptions->dwSleepClose)
                Sleep(pOptions->dwSleepClose);
            if (INVALID_SOCKET != pSendInfo->sd)
            {
                closesocket(pSendInfo->sd);
                pSendInfo->sd = INVALID_SOCKET;
            }
            bProcessEventsDone = TRUE;
            WSAResetEvent(pSendInfo->hEvents[WSASEND_EVENT]);
        }
    }

    return bProcessEventsDone;
}



