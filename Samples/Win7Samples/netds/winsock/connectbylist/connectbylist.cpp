/******************************************************************************\
* connectbylist.cpp
*
* This IPv6 sample demonstrates the use of WSAConnectByList and dual IPv4/IPv6 
* sockets.
*
* WSAConnectByList is new to Windows Sockets in Windows Vista.  
*
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista).
*
* The client builds a socket address list and connects to the server. This is done
* for IPv4 (IPv6 v4-mapped address) and IPv6. 
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


//safe macros
#define MALLOC(x) \
        HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,x)

#define FREE(p) \
        if(NULL != p) {HeapFree(GetProcessHeap(),0,p); p = NULL;}

#define MSIZE(p) \
        HeapSize(GetProcessHeap(),0,p)

#define CLOSESOCK(s) \
        if(INVALID_SOCKET != s) {closesocket(s); s = INVALID_SOCKET;}
        
#define FREEADDRINFO(p) \
        if(NULL != p) {freeaddrinfo(p); p = NULL;}

#define ERR(e) \
        printf("%s:%s failed: %d [%s@%ld]\n",__FUNCTION__,e,WSAGetLastError(),__FILE__,__LINE__)

//constants
#define WS_VER              0x0202
#define CONNECT_TIMEOUT_VAL 5000

USHORT g_listenPort = 0;

//helper functions
typedef BOOL (WSAAPI * LPFN_WSACONNECTBYLIST) (SOCKET s,
                                               PSOCKET_ADDRESS_LIST SocketAddress,
                                               LPDWORD LocalAddressLength,
                                               LPSOCKADDR LocalAddress,
                                               LPDWORD RemoteAddressLength,
                                               LPSOCKADDR RemoteAddress,
                                               TIMEVAL * timeout,
                                               LPWSAOVERLAPPED Reserved
                                               );

LPFN_WSACONNECTBYLIST GetWSAConnectByListFunctionPointer()
{
    LPFN_WSACONNECTBYLIST fnWSAConnectByList = NULL;
    HMODULE hWs2_32 = LoadLibraryA("ws2_32");

    fnWSAConnectByList = (LPFN_WSACONNECTBYLIST)GetProcAddress(hWs2_32,"WSAConnectByList");

    FreeLibrary(hWs2_32);

    return fnWSAConnectByList;
}

LPFN_WSACONNECTBYLIST fnWSAConnectByList = GetWSAConnectByListFunctionPointer();

//Dual-stack socket which can be used with either IPv4 or IPv6 addresses
SOCKET DualStackSocket(VOID)
{
    SOCKET  sock = INVALID_SOCKET;
    INT     off = 0;

    __try
    {
        if (INVALID_SOCKET == (sock = socket(AF_INET6,SOCK_STREAM,0)))
        {
            ERR(TEXT("socket"));
            __leave;
        }

        if (SOCKET_ERROR == setsockopt(sock,
                                       IPPROTO_IPV6,
                                       IPV6_V6ONLY,
                                       (char*)&off,
                                       sizeof off
                                       ))
        {
            ERR(TEXT("setsockopt"));
            CLOSESOCK(sock);
            __leave;
        }
    }
    __finally
    {
    }

    return sock;
}

SOCKET DualStackListenSocket(VOID)
{
    SOCKET              lsock = INVALID_SOCKET;
    SOCKADDR_STORAGE    addrAny = {0};
    INT                 addrAny_len = 0;

    __try
    {
        addrAny.ss_family = AF_INET6;

        INETADDR_SETANY((SOCKADDR*)&addrAny);

        //dual stack listening socket
        
        if (INVALID_SOCKET == (lsock = DualStackSocket()))
        {
            
            ERR(TEXT("DualStackSocket"));
            __leave;
        }

        if (SOCKET_ERROR == bind(lsock,(SOCKADDR*)&addrAny,sizeof addrAny))
        {
            ERR(TEXT("bind"));
            CLOSESOCK(lsock);
            __leave;
        }

        if (SOCKET_ERROR == listen(lsock,SOMAXCONN))
        {
            ERR(TEXT("listen"));
            CLOSESOCK(lsock);
            __leave;
        }

        addrAny_len = sizeof addrAny;

        if (SOCKET_ERROR == getsockname(lsock,(SOCKADDR*)&addrAny,&addrAny_len))
        {
            ERR(TEXT("getsockname"));
            CLOSESOCK(lsock);
            __leave;
        }

        g_listenPort = htons(INETADDR_PORT((SOCKADDR*)&addrAny));

    }
    __finally
    {
    }

    return lsock;

}

SOCKADDR_IN6 *pAddr6 = NULL;

PSOCKET_ADDRESS_LIST InitSocketAddressList(ADDRESS_FAMILY Af)
{
    BOOL                    bRet = FALSE;
    CHAR                    szComputerName[MAX_COMPUTERNAME_LENGTH+1] = {0},
                            szPort[6] = {0};
    DWORD                   dwSize = sizeof szComputerName,
                            i = 0, 
                            dwAddrCount = 0,
                            dwBytes = 0;
    ADDRINFO                hints = {0},
                            *res = NULL,
                            *ptr = NULL;
    SOCKET                  sock = INVALID_SOCKET;
    SOCKADDR_IN6            *pAddr6 = NULL;
    PSOCKET_ADDRESS_LIST    SocketAddressList = NULL;
    SCOPE_ID                scope;

    __try
    {
        if (!GetComputerNameA(szComputerName,&dwSize))
        {
            ERR("GetComputerName");
            __leave;
        }

        hints.ai_family = (INT)Af;
        hints.ai_socktype = SOCK_STREAM;


        if (FAILED(StringCbPrintfA(szPort,sizeof szPort,"%d",g_listenPort)))
        {
            __leave;
        }

        if (GetAddrInfoA(szComputerName,"",&hints,&res))
        {
            ERR("getaddrinfo");
            __leave;
        }

        for (ptr = res; ptr; ptr = ptr->ai_next)
            dwAddrCount += 1;

        dwSize = SIZEOF_SOCKET_ADDRESS_LIST(dwAddrCount)+(dwAddrCount*sizeof SOCKADDR_STORAGE);;

        if (NULL == (SocketAddressList = (PSOCKET_ADDRESS_LIST)MALLOC(dwSize)))
        {
            ERR("HeapAlloc");
            __leave;
        }

        pAddr6 = NULL;

        if (NULL == (pAddr6 = (SOCKADDR_IN6*)MALLOC(dwAddrCount*sizeof SOCKADDR_IN6)))
        {
            ERR("HeapAlloc");
            __leave;
        }

        if (AF_INET == res->ai_family)
        {


            for (ptr = res,i = 0; ptr, i < dwAddrCount; ptr = ptr->ai_next, i++)
            {
                scope = INETADDR_SCOPE_ID(ptr->ai_addr);

                IN6ADDR_SETV4MAPPED(&pAddr6[i],
                                    &((SOCKADDR_IN*)ptr->ai_addr)->sin_addr,
                                    scope,
                                    ((SOCKADDR_IN*)ptr->ai_addr)->sin_port
                                   );

                SS_PORT((SOCKADDR*)&pAddr6[i]) = htons(g_listenPort);

                SocketAddressList->Address[i].lpSockaddr = (SOCKADDR*)&pAddr6[i];
                SocketAddressList->Address[i].iSockaddrLength = sizeof SOCKADDR_IN6;

            }
        } else
        {
            for (ptr = res, i = 0; ptr, i < dwAddrCount; ptr = ptr->ai_next, i++)
            {
                CopyMemory(&pAddr6[i],(SOCKADDR_IN6*)ptr->ai_addr,sizeof SOCKADDR_IN6);

                SS_PORT((SOCKADDR*)&pAddr6[i]) = htons(g_listenPort);

                SocketAddressList->Address[i].lpSockaddr = (SOCKADDR*)&pAddr6[i];
                SocketAddressList->Address[i].iSockaddrLength = sizeof SOCKADDR_IN6;

            }

        }

        SocketAddressList->iAddressCount = dwAddrCount;

        if (INVALID_SOCKET == (sock = socket(AF_INET6,
                                             SOCK_STREAM,
                                             0
                                            )))
        {
            ERR("socket");
            __leave;
        }

        dwSize = (DWORD)MSIZE(SocketAddressList);

        if (SOCKET_ERROR == WSAIoctl(sock,
                                     SIO_ADDRESS_LIST_SORT,
                                     SocketAddressList,
                                     dwSize,
                                     SocketAddressList,
                                     dwSize,
                                     &dwBytes,
                                     NULL,
                                     NULL
                                    ))
        {
            ERR("SIO_ADDRESS_LIST_SORT");
            __leave;
        }


        bRet = TRUE;

    }
    __finally
    {
        CLOSESOCK(sock);
        FREEADDRINFO(res);
        
    }

    if (!bRet)
    {
        FREE(SocketAddressList);
    }

    return SocketAddressList;

}

VOID PrintSocketAddressList(PSOCKET_ADDRESS_LIST List)
{
    CHAR    szString[MAX_PATH] = {0};
    DWORD   dwLen = 0;
    INT     i=0;

    printf("SOCKET_ADDRESS_LIST:\n");

    for (i=0; i < List->iAddressCount; i++)
    {
        dwLen = sizeof szString;

        if(SOCKET_ERROR == WSAAddressToStringA(List->Address[i].lpSockaddr,
                           (DWORD)List->Address[i].iSockaddrLength,
                           NULL,
                           szString,
                           &dwLen
                           ))
        {
            printf("WSAAddressToString: %d\n",WSAGetLastError());
        }

        printf("%s\n",szString);

    }
}

VOID PrintSockaddr(PSOCKADDR pSockaddr,DWORD dwSize)
{
    CHAR    szString[MAX_PATH] = {0};
    DWORD   dwStringSize = sizeof szString;

    if (SOCKET_ERROR == WSAAddressToStringA(pSockaddr,dwSize,NULL,szString,&dwStringSize))
    {
        ERR("WSAAddressToString");
        return;
    }

    printf("%s\n",szString);

    return;

}

DWORD WSAConnectByList_Loopback(ADDRESS_FAMILY AddressFamily)
{

    WSADATA                 wsd;
    INT                     nStartup = 0,
                            nErr = 0,
                            rc = 0;
    SOCKET                  lsock = INVALID_SOCKET,
                            asock = INVALID_SOCKET,
                            csock = INVALID_SOCKET;
    TIMEVAL                 timeval = {CONNECT_TIMEOUT_VAL,0};
    PSOCKET_ADDRESS_LIST    AddressList = NULL;
    CHAR                    buf = '\0';
    SOCKADDR_STORAGE        localAddr = {0},
                            remoteAddr = {0};
    DWORD                   dwLocalAddrLen = sizeof localAddr,
                            dwRemoteAddrLen = sizeof remoteAddr;

    __try
    {

        nErr = WSAStartup(WS_VER,&wsd);
        if (nErr)
        {
            WSASetLastError(nErr);
            ERR("WSAStartup");
            __leave;
        }
        nStartup++;

        if (NULL == fnWSAConnectByList)
        {
            printf("Not supported\n");
            __leave;

        }

        if (INVALID_SOCKET == (lsock = DualStackListenSocket()))
        {
            ERR("DualStackListenSocket");
            __leave;
        }

        if ((NULL == (AddressList = InitSocketAddressList(AddressFamily))))
        {
            ERR("InitSocketAddressList");
            __leave;
        }
        
        if (INVALID_SOCKET == (csock = DualStackSocket()))
        {
            ERR("DualStackSocket");
            __leave;

        }

        PrintSocketAddressList(AddressList);

        if (!fnWSAConnectByList(csock,
                                AddressList,
                                &dwLocalAddrLen,
                                (SOCKADDR*)&localAddr,
                                &dwRemoteAddrLen,
                                (SOCKADDR*)&remoteAddr,
                                &timeval,
                                NULL
                               ))
        {
            ERR("WSAConnectByList");
            __leave;
        }

        printf("WSAConnectByList returned\n");
        printf("Local address ");
        PrintSockaddr((SOCKADDR*)&localAddr,dwLocalAddrLen);
        printf("Remote address ");
        PrintSockaddr((SOCKADDR*)&remoteAddr,dwRemoteAddrLen);


        if (INVALID_SOCKET == (asock = accept(lsock,NULL,NULL)))
        {
            ERR("accept");
            __leave;
        }

        if (SOCKET_ERROR == shutdown(asock,SD_SEND))
        {
            ERR("shutdown");
            __leave;
        }

        if (0 != (rc = recv(csock,&buf,sizeof buf,0)))
        {
            ERR("recv");
            __leave;
        }

    }
    __finally
    {

        CLOSESOCK(csock);
        CLOSESOCK(asock);
        CLOSESOCK(lsock);
        FREE(pAddr6);
        FREE(AddressList);
        if (nStartup) WSACleanup();
    }

    return 0;

}

int __cdecl main()
{
    ADDRESS_FAMILY Af = AF_UNSPEC;

    Af = (ADDRESS_FAMILY)AF_INET;

    WSAConnectByList_Loopback(Af);

    Af = (ADDRESS_FAMILY)AF_INET6;

    WSAConnectByList_Loopback(Af);

    return 0;
    
}
