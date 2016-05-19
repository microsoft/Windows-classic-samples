// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993 - 2000   Microsoft Corporation.
// All Rights Reserved.
//
// Written By:    Mike Liu
//

#ifdef _IA64_
	#pragma warning(disable:4127 4706 4267)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

char *DEFAULT_PORT                = "8765";
const int DEFAULT_ITERATIONS      = 10;
const int LOOP_FOREVER            = -1;
SOCKET gsConnect = INVALID_SOCKET;

BOOL ConsoleCtrlHandler(DWORD dwEvent);
BOOL DoGracefulShutdown(SOCKET sock);

void Usage(char *szProgramName)
{
   fprintf(stderr, "Usage: %s [-n Server] [-e Endpoint] [-l Iterations]\n\n", szProgramName);
   fprintf(stderr, "   Server:     server name or IP address (default localhost)\n");
   fprintf(stderr, "   Endpoint:   port to connect to (default %s)\n", DEFAULT_PORT);
   fprintf(stderr, "   Iterations: number of messages to send (default %d)\n", DEFAULT_ITERATIONS);
}

#pragma warning(push)
#pragma warning(disable:4127)

int __cdecl main(int argc, char **argv)
{
    char szBuf[MAX_PATH+1];
    char *pszServerName = "localhost";
    char *pszPort = DEFAULT_PORT;
    int i, nStatus, nLoopCount;
    int nIterations = DEFAULT_ITERATIONS;
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    WSADATA wsaData;
    WSABUF wsaBuf;
    DWORD dwSent = 0;
    DWORD dwFlags = 0;

    if (argc > 1)
    {
        for(i = 1; i < argc; i++)
        {
            if ((argv[i][0] == '-') || (argv[i][0] == '/'))
            {
                switch(tolower(argv[i][1]))
                {
                    case 'n':
                        if (i + 1 < argc)
                            pszServerName = argv[++i];
                        break;

                    case 'e':
                        if (i + 1 < argc)
                            pszPort = argv[++i];
                        break;

                    case 'l':
                        if (i + 1 < argc && argv[i+1])
                        {
                            if (argv[i+1][0] != '-')  // "-l 6"
                            {
                                nIterations = atoi(argv[i+1]);
                                i++;
                            }
                            else if (isdigit(argv[i+1][1])) // "-l -6"
                            {
                                nIterations = 0;
                                i++;
                            }
                            else // "-l -e 2000"
                                nIterations = LOOP_FOREVER;
                        }
                        else // "-e 2000 -l"
                            nIterations = LOOP_FOREVER;

                        break;

                    default:
                        Usage(argv[0]);
                        exit(-1);
                }
            }
            else
            {
                Usage(argv[0]);
                exit(-1);
            }
        }
    }

    if (nIterations == 0)
        exit(-1);

    // Install the CTRL+BREAK and CTRL+C Handler
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler,
                TRUE)
            == FALSE) 
        fprintf(stderr, "SetConsoleCtrlHandler failed: %d", GetLastError());

    if ((nStatus = WSAStartup(0x202,&wsaData)) != 0)
    {
        fprintf(stderr, "\nWinsock 2 DLL initialization failed: %d\n", nStatus);
        exit(-1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //  getaddrinfo: declared in winsock2.h import library is ws2_32.lib
    if (getaddrinfo(pszServerName, pszPort, &hints, &res) != 0)
    {
        fprintf(stderr, "getaddrinfo failed for gListenSocket: %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    if (res == NULL)
    {
        fprintf(stderr, "No matching addresses found. "
                "getaddrinfo returned res = NULL\n");
        goto CLEANUP;
    }	

    gsConnect = WSASocket(
            res->ai_family, 
            res->ai_socktype, 
            res->ai_protocol,
            NULL,
            0,
            0);

    if (gsConnect == INVALID_SOCKET)
    {
        fprintf(stderr, "\nWSASocket() failed: %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    printf("Connecting to: %s ...\n", pszServerName);

    if (connect(gsConnect, (struct sockaddr*)res->ai_addr, (int) res->ai_addrlen)
            == SOCKET_ERROR)
    {
        fprintf(stderr, "\nconnect() failed: %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    nLoopCount = 0;

    while (TRUE)
    {
        szBuf[0] = '\0';
        StringCbPrintf(szBuf, sizeof szBuf,"Test message %d\n", nLoopCount++);
        wsaBuf.len = lstrlen(szBuf);
        wsaBuf.buf = (char *)szBuf;

        printf("Sending --> %s", szBuf);

        nStatus = WSASend(gsConnect,
                &wsaBuf,
                1,
                &dwSent,
                dwFlags,
                (LPWSAOVERLAPPED) NULL,
                0);

        if (nStatus == SOCKET_ERROR)
        {
            fprintf(stderr, "WSASend() failed: %d\n", WSAGetLastError());
            break;
        }
        else if (dwSent == 0)
        {
            printf("Server closed connection\n");
            break;
        }

        if (((nIterations != LOOP_FOREVER)) && (nLoopCount >= nIterations))
            break;
    }

CLEANUP:

    if (gsConnect != INVALID_SOCKET)
    {
        DoGracefulShutdown(gsConnect);
        gsConnect = INVALID_SOCKET;
    }

    if (res != NULL)
    {
        // return the addrinfo structure allocated for us by getaddrinfo.
        freeaddrinfo(res);
        res = NULL;
    }

    WSACleanup();
    return 0;
}

#pragma warning(pop)


// Handle CTRL+BREAK or CTRL+C events
BOOL ConsoleCtrlHandler(DWORD dwEvent)
{
    UNREFERENCED_PARAMETER( dwEvent );

    // Do a best effort cleanup
    if (gsConnect != INVALID_SOCKET)
    {
        closesocket(gsConnect);
        gsConnect = INVALID_SOCKET;
    }
    WSACleanup();
    printf("Client terminated. Bye!\n");

    return FALSE;
}


// Do a graceful shutdown of a the given socket sock.
// as per the documentation of the shutdown API.
BOOL DoGracefulShutdown(SOCKET sock)
{
    BOOL bRetVal = FALSE;
    WSAEVENT hEvent = WSA_INVALID_EVENT;
    long lNetworkEvents = 0;
    int status = 0;

    hEvent = WSACreateEvent();
    if (hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, "DoGracefulShutdown: WSACreateEvent failed: %d\n", 
                WSAGetLastError());
        goto CLEANUP;
    }

    lNetworkEvents = FD_CLOSE;
    if (WSAEventSelect(sock, hEvent, lNetworkEvents) != 0)
    {
        fprintf(stderr, "DoGracefulShutdown: WSAEventSelect failed: %d\n", 
                WSAGetLastError());
        goto CLEANUP;
    }

    if (shutdown(sock, SD_SEND) != 0)
    {
        fprintf(stderr, "DoGracefulShutdown: shutdown failed: %d\n", 
                WSAGetLastError());
        goto CLEANUP;
    }

    if (WaitForSingleObject(hEvent, INFINITE) != WAIT_OBJECT_0)
    {
        fprintf(stderr, "DoGracefulShutdown: WaitForSingleObject failed: %d\n", 
                WSAGetLastError());
        goto CLEANUP;
    }

    do 
    {
        char buf[128];

        status = recv(sock, buf, sizeof(buf), 0);
    } while (!(status == 0 || status == SOCKET_ERROR));

    if (closesocket(sock) != 0)
    {
        fprintf(stderr, "DoGracefulShutdown: closesocket failed: %d\n",
                WSAGetLastError());
        goto CLEANUP;
    }

    printf("Socket %d has been closed gracefully\n", sock);
    sock = INVALID_SOCKET;
    bRetVal = TRUE;

CLEANUP:

    if (hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(hEvent);
        hEvent = WSA_INVALID_EVENT;
    }

    if (sock != INVALID_SOCKET)
    {
        fprintf(stderr, "DoGracefulShutdown: Can't close socket gracefully. "
                "So, closing it anyway ... \n");
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    return bRetVal;
}
