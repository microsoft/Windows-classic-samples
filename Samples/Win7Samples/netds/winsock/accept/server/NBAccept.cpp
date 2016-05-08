// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: NBAccept.cpp
//
// Description:
//             This file contains the functions for implementing the 
// non-blocking version of the accept. It uses many of the common functions
// defined in Common.cpp.

#pragma warning (disable: 4127)

#include "common.h"

/*
    This function is the entry point for the Non-blocking Accept implementation.
    It waits on select for each of the listening/accepted sockets until any
    event is signalled. If a listening socket gets signalled, it performs 
    accept. If an accepted socket gets signalled, it performs read and echoes
    back the data.
*/
void NonBlockingAcceptMain()
{
    // prints a "i am still awake" message every few seconds.
    const long HEART_BEAT_INTERVAL = 30;     
    PSOCK_INFO pSockInfo,
               pNextSock;
    FD_SET readFDSet;
    struct timeval interval;
    int nReady;
    BOOL bSocketError;
    int nSocksInFDSet;
    
    printf("Entering NonBlockingAcceptMain()\n");

    // process the available sockets in the global socket list.
    while(g_AcceptContext.pSockList)
    {
        // first, prepare the FD_SET for passing to select.

        // initialize the FD_SET
        FD_ZERO(&readFDSet);
        nSocksInFDSet = 0;

        // include each socket in the FD_SET. For listening sockets,
        // read means a new connection is available.
        for(pSockInfo = g_AcceptContext.pSockList;  pSockInfo != NULL; 
                                                    pSockInfo = pSockInfo->next)
        {
            // the FD_SET will take only upto FD_SETSIZE number of sockets.
            // so, we signal an error if we exceed the MAX_CLIENTS which is
            // less than or equal to FD_SETSIZE. If more clients need to be
            // supported, then FD_SETSIZE can be redefined before including
            // winsock2.h
            if (nSocksInFDSet < MAX_CLIENTS)
            {
                FD_SET(pSockInfo->sock, &readFDSet);
                nSocksInFDSet++;
            }
            else
            {
                printf("ERROR: Number of sockets in FD Set exceeds MAX_CLIENTS (%d)\n", MAX_CLIENTS);
                break;
            }
        }
        printf("Num sockets in FD_SET: %d\n", nSocksInFDSet);

        // wait on select for HEART_BEAT_INTEVAL seconds to see if any of
        // the sockets are signalled.
        interval.tv_sec = HEART_BEAT_INTERVAL;
        interval.tv_usec = 0;
        printf("Waiting in select for data/connections ...\n");

        // currently we use only the read set, which signals all the
        // three events required:
        //      accept for listening sockets, 
        //      read for connected sockets and
        //      close for both type of sockets on error.
        nReady = select(0, &readFDSet, NULL, NULL, &interval);
        if (nReady == SOCKET_ERROR)
        {
            printf("ERROR: select failed. Error = %d\n", WSAGetLastError());

            // pause so as not to output frequently in case of repeated errors.        
            Sleep(3000); 
            continue;
        }

        // check if some socket was signalled.
        if (nReady == 0)
        {
            printf("No connections/data in the last %d seconds. \n",
                                                HEART_BEAT_INTERVAL);
            continue;
        }

        // find out which socket was signalled and process that socket.
        // since nReady indicates how many sockets are in the signalled
        // state, we don't need to loop if we have already processed that
        // many sockets.
        pSockInfo = g_AcceptContext.pSockList;  
        while (pSockInfo != NULL && nReady > 0)
        {
            // check if this socket is set.
            if (FD_ISSET(pSockInfo->sock, &readFDSet))
            {
                nReady--;

                // for listening sockets, signalling on a read set means
                // a new connection is available.
                if (pSockInfo->isSocketListening)
                {
                    // accept the connection and add the new socket to the
                    // beginning of the list (not the end, so that we won't
                    // come across the newly added sockets before they are
                    // included in the next select call later).
                    ProcessAcceptEvent(pSockInfo);
                }
                else
                {
                    // this read event for a connected socket means data
                    // is available or socket is closed.

                    // read data if available and if we have buffer space.
                    bSocketError = ProcessReadEvent(pSockInfo);

                    // if we already found that the socket is closed due to
                    // some errors, we don't need to send the data
                    if (!bSocketError)
                    {
                        // so far no error, so try echoing the data back.
                        bSocketError = SendData(pSockInfo);
                    }

                    // if there was an error in recv/send, close the
                    // socket.
                    if (bSocketError)
                    {
                        // close the socket 
                        closesocket(pSockInfo->sock);
                        printf("Closed socket %d. "
                               "Total Bytes Recd = %d, "
                               "Total Bytes Sent = %d\n",
                                pSockInfo->sock, 
                                pSockInfo->nTotalRecd,
                                pSockInfo->nTotalSent);

                        // delete the SockInfo structure and free the memory.
                        pNextSock = pSockInfo->next;
                        DeleteSockInfoFromList(&g_AcceptContext.pSockList,
                                               pSockInfo);
                        pSockInfo = pNextSock;
                        continue;
                    }
                }
            }

            // move on to the next socket. as mentioned earlier, since
            // new sockets are added only in the beginning of the list
            // we won't be processing them until they are included in the
            // next select call.
            pSockInfo = pSockInfo->next;
        }

    }

    printf("Exiting NonBlockingAcceptMain()\n");
    return;
}
