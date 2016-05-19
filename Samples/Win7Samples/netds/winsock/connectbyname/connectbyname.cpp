/******************************************************************************\
* connectbyname.cpp
*
* This IPv6 sample demonstrates the use of WSAConnectByName.
*
* WSAConnectByName is new to Windows Sockets in Windows Vista.
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista).
*
* IPv6 socket calls WSAConnectByName to resolve name and connect to listener in 
* a single API call. The local and remote addresses are also returned. The connecting
* entity sends some data after WSAConnectByName. The accepting entity receives the
* peer data. Then both sides close the connection. 
*
*
* This is a part of the Microsoft Source Code Samples.
* Copyright 1996 - 2006 Microsoft Corporation.
* All rights reserved.
* This source code is only intended as a supplement to
* Microsoft Development Tools and/or WinHelp documentation.
* See these sources for detailed information regarding the
* Microsoft samples programs.
\******************************************************************************/

#ifdef _IA64_
    #pragma warning (disable: 4311)
    #pragma warning (disable: 4312)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <stdio.h>
#include <strsafe.h>


// "Safe" macros

#define CLOSESOCK(s) \
        if(INVALID_SOCKET != s) {closesocket(s); s = INVALID_SOCKET;}

#define ERR(e) \
        { \
        printf("%s:%s failed: %d [%s@%ld]\n",__FUNCTION__,e,WSAGetLastError(),__FILE__,__LINE__); \
        }


// Constants

#define WS_VER 0x0202

#define CONNECT_TIMEOUT_VAL 30000

#define TST_MSG "Hello\0"

// Helper functions

VOID SET_PORT(PSOCKADDR pAddr,USHORT port)
{
    if (AF_INET6 == pAddr->sa_family)
    {
        ((SOCKADDR_IN6*)pAddr)->sin6_port = htons(port);
    }
    else
    {
        ((SOCKADDR_IN*)pAddr)->sin_port = htons(port);
    }
}


typedef BOOL (WSAAPI * LPFN_WSACONNECTBYNAME) (SOCKET s,
                                               LPCSTR name,
                                               LPCSTR port,
                                               LPDWORD LocalAddressLength,
                                               LPSOCKADDR LocalAddress,
                                               LPDWORD RemoteAddressLength,
                                               LPSOCKADDR RemoteAddress,
                                               TIMEVAL* timeout,
                                               LPOVERLAPPED reserved);

LPFN_WSACONNECTBYNAME GetWSAConnectByNameFunctionPointer()
{
    LPFN_WSACONNECTBYNAME WSAConnectByName = NULL;
    HMODULE hWs2_32 = LoadLibraryA("ws2_32");

    WSAConnectByName = (LPFN_WSACONNECTBYNAME)GetProcAddress(hWs2_32,"WSAConnectByNameA");

    FreeLibrary(hWs2_32);

    return WSAConnectByName;
}

VOID PrintSockaddr(PSOCKADDR pSockaddr,DWORD dwSize)
{
    CHAR szString[MAX_PATH] = {0};
    DWORD dwStringSize = sizeof szString;

    if (SOCKET_ERROR == WSAAddressToStringA(pSockaddr,dwSize,NULL,szString,&dwStringSize))
    {
        ERR("WSAAddressToString");
        return;
    }

    printf("%s\n",szString);

    return;

}


int __cdecl main()
{
    WSADATA                 wsd;
    INT                     nStartup = 0,
                            err = 0,
                            nAddrSize = 0,
                            rc = 0;
    SOCKET                  lsock = INVALID_SOCKET,
                            csock = INVALID_SOCKET,
                            asock = INVALID_SOCKET;
    SOCKADDR_STORAGE        listenAddr = {0},
                            localAddr = {0},
                            remoteAddr = {0};
    USHORT                  uport = 0;
    CHAR                    szPort[MAX_PATH] = {0},
                            hostname[MAX_PATH] = {0},
                            buf[MAX_PATH] = {0}; 
    DWORD                   dwLocalAddrLen = sizeof localAddr,
                            dwRemoteAddrLen = sizeof remoteAddr;
    TIMEVAL                 timeval = {CONNECT_TIMEOUT_VAL,0};
    LPFN_WSACONNECTBYNAME   WSAConnectByName = GetWSAConnectByNameFunctionPointer();

    __try
    {
        err = WSAStartup(WS_VER,&wsd);
        if (err)
        {
            WSASetLastError(err);
            ERR("WSAStartup");
            __leave;
        }
        else
            nStartup++;

        //Setup listener
        if (INVALID_SOCKET == (lsock = socket(AF_INET6,
                                              SOCK_STREAM,
                                              0
                                              )))
        {
            ERR("socket");
            __leave;
        }

        listenAddr.ss_family = (ADDRESS_FAMILY)AF_INET6;

        INETADDR_SETANY((SOCKADDR*)&listenAddr);

        if (SOCKET_ERROR == bind(lsock,
                                 (SOCKADDR*)&listenAddr,
                                 sizeof listenAddr
                                 ))
        {
            ERR("bind");
            __leave;
        }

        nAddrSize = sizeof listenAddr;

        if (SOCKET_ERROR == getsockname(lsock,
                                        (SOCKADDR*)&listenAddr,
                                        &nAddrSize
                                        ))
        {
            ERR("getsockname");
            __leave;
        }

        //get listening port
        uport = htons((USHORT)INETADDR_PORT((SOCKADDR*)&listenAddr));

        //convert uport to string for WSAConnectByName
        StringCbPrintfA(szPort,sizeof szPort,"%u",uport);

        printf("Listening on port %s\n",szPort);


        if (SOCKET_ERROR == listen(lsock,
                                   1
                                   ))
        {
            ERR("listen");
            __leave;
        }

        //client WSAConnectByName

        if (INVALID_SOCKET == (csock = socket(AF_INET6,
                                              SOCK_STREAM,
                                              0
                                              )))
        {
            ERR("socket");
            __leave;
        }

        if(SOCKET_ERROR == gethostname(hostname,sizeof hostname))
        {
            ERR("gethostname");
            __leave;
        }

        if (!WSAConnectByName(csock,
                              hostname,
                              szPort,
                              &dwLocalAddrLen,
                              (SOCKADDR*)&localAddr,
                              &dwRemoteAddrLen,
                              (SOCKADDR*)&remoteAddr,
                              &timeval,
                              NULL
                              ))
        {
            ERR("WSAConnectByName");
            __leave;
        }

        //send some data

        if (SOCKET_ERROR == (rc = send(csock,
                                       TST_MSG,
                                       lstrlenA(TST_MSG),
                                       0
                                       )))
        {
            ERR("send");
            __leave;
        }

        printf("Sent %d bytes to listener after WSAConnectByName.\n",rc);

        printf("Addresses returned after WSAConnectByName:\n");
        printf("Local address: ");
        PrintSockaddr((SOCKADDR*)&localAddr,sizeof localAddr);
        printf("Remote address: ");
        PrintSockaddr((SOCKADDR*)&remoteAddr,sizeof remoteAddr);
        printf("\n");

        //accept connection
        if (INVALID_SOCKET == (asock = accept(lsock,
                                              NULL,
                                              NULL
                                              )))
        {
            ERR("accept");
            __leave;
        }

        //recv client data
        if (SOCKET_ERROR == (rc = recv(asock,
                                       buf,
                                       sizeof buf,
                                       0
                                       )))
        {
            ERR("recv");
            __leave;
        }

        printf("Received %d bytes after accept.\n",rc);
        
    }
    __finally
    {
        CLOSESOCK(lsock);
        CLOSESOCK(csock);
        CLOSESOCK(asock);
        if(nStartup) WSACleanup();
    }

    return 0;
}
