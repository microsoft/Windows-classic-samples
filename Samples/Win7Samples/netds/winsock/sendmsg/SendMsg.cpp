/******************************************************************************\
* sendmsg.cpp
*
* This IPv6 sample demonstrates the use of WSASendMsg() and the IPV6_PKTINFO option
* and WSARecvMsg to retrieve the source address of a datagram.
*
* WSASendMsg is new to Windows Sockets in Windows Vista.
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista).
*
* A IPv6 datagram socket is created and bound and the IPV6_PKTINFO option is set. 
* Overlapped WSARecvMsg is posted, and data is sent on the socket via sendto. 
* Upon completion, the received message is sent again via WSASendMsg.
* The data is received again via WSARecvMsg. 
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
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <stdlib.h>

// "Safe" macros


#define MALLOC(x) \
        HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,x)

#define FREE(p) \
        if(NULL != p) {HeapFree(GetProcessHeap(),0,p); p = NULL;}

#define MSIZE(p) \
        HeapSize(GetProcessHeap(),0,p)

#define CLOSESOCK(s) \
        if(INVALID_SOCKET != s) {closesocket(s); s = INVALID_SOCKET;}

#define CLOSESOCKEVENT(h) \
        if(WSA_INVALID_EVENT != h) {WSACloseEvent(h); h = WSA_INVALID_EVENT;}
        
#define ERR(e) \
        { \
        printf("%s:%s failed: %d [%s@%ld]\n",__FUNCTION__,e,WSAGetLastError(),__FILE__,__LINE__); \
        }


// Constants

#define WS_VER 0x0202

#define DEFAULT_PORT 12345

#define DEFAULT_WAIT 5000

#define DATA_SIZE 100

#define TST_MSG "Hello\0"

#define IN6_ADDR_STRING_LEN 65

// Helper functions

VOID SET_PORT(PSOCKADDR pAddr,USHORT port)
{
    if (AF_INET6 == pAddr->sa_family)
    {
        ((SOCKADDR_IN6*)pAddr)->sin6_port = port;
    }
    else
    {
        ((SOCKADDR_IN*)pAddr)->sin_port = port;
    }
}

VOID InitOverlap(LPWSAOVERLAPPED pOver)
{
    CLOSESOCKEVENT(pOver->hEvent);

    ZeroMemory(pOver,sizeof(WSAOVERLAPPED));

    if (WSA_INVALID_EVENT == (pOver->hEvent = WSACreateEvent()))
    {
        ERR("WSACreateEvent");
    }
}

BOOL SetIpv6PktInfoOption(SOCKET sock)
{
    DWORD dwEnableOption = 1;

    if (SOCKET_ERROR == setsockopt(sock,
                                   IPPROTO_IPV6,
                                   IPV6_PKTINFO,
                                   (CHAR*)&dwEnableOption,
                                   sizeof dwEnableOption
                                   ))
    {
        ERR("setsockopt IPV6_PKTINFO"); 
        return FALSE;
    }

    return TRUE;

}

BOOL AllocAndInitIpv6PktInfo(LPWSAMSG pWSAMsg)
{
    PBYTE CtrlBuf = (PBYTE)MALLOC(WSA_CMSG_SPACE(sizeof IN6_PKTINFO)); //caller frees heap allocated CtrlBuf

    if (NULL == CtrlBuf)
    {
        ERR("HeapAlloc");
        return FALSE;
    }
    
    pWSAMsg->Control.buf = (CHAR*)CtrlBuf;
    pWSAMsg->Control.len = (ULONG)MSIZE(CtrlBuf);

    return TRUE;
}

VOID SockaddrToString(PSOCKADDR pSockaddr,DWORD dwSize,LPSTR szString,DWORD dwStringSize)
{
    if (SOCKET_ERROR == WSAAddressToStringA(pSockaddr,dwSize,NULL,szString,&dwStringSize))
    {
        ERR("WSAAddressToString");
        return;
    }

    return;
}

INT ProcessIpv6Msg(LPWSAMSG pWSAMsg)
{
    PWSACMSGHDR pCtrlInfo = NULL;
    PIN6_PKTINFO pPktInfo = NULL;
    CHAR szAddr[IN6_ADDR_STRING_LEN] = {0};
    SOCKADDR_STORAGE addr = {0};
    DWORD dwAddrStrSize = 0;
    INT rc = SOCKET_ERROR;


    pCtrlInfo = WSA_CMSG_FIRSTHDR(pWSAMsg);

    if ((IPPROTO_IPV6 == pCtrlInfo->cmsg_level) &&
        (IPV6_PKTINFO == pCtrlInfo->cmsg_type))
    {
        pPktInfo = (PIN6_PKTINFO)WSA_CMSG_DATA(pCtrlInfo);

        ((SOCKADDR_IN6*)&addr)->sin6_family = AF_INET6;
        ((SOCKADDR_IN6*)&addr)->sin6_addr = pPktInfo->ipi6_addr;

        dwAddrStrSize = sizeof szAddr;
        if(SOCKET_ERROR == (rc = WSAAddressToStringA((SOCKADDR*)&addr,
                                                    sizeof addr,
                                                    NULL,
                                                    szAddr,
                                                    &dwAddrStrSize
                                                    )))
        {
            ERR("WSAAddressToString");
        }

        printf("ProcessIpv6Msg: \npPktInfo->ipi6_addr: %s\npPktInfo->ipi6_ifindex: %d\n",
               szAddr,pPktInfo->ipi6_ifindex);

        return rc;

    }

    return rc;
}

LPFN_WSARECVMSG GetWSARecvMsgFunctionPointer()
{
    LPFN_WSARECVMSG     lpfnWSARecvMsg = NULL;
    GUID                guidWSARecvMsg = WSAID_WSARECVMSG;
    SOCKET              sock = INVALID_SOCKET;
    DWORD               dwBytes = 0;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(SOCKET_ERROR == WSAIoctl(sock, 
                                SIO_GET_EXTENSION_FUNCTION_POINTER, 
                                &guidWSARecvMsg, 
                                sizeof(guidWSARecvMsg), 
                                &lpfnWSARecvMsg, 
                                sizeof(lpfnWSARecvMsg), 
                                &dwBytes, 
                                NULL, 
                                NULL
                                ))
    {
        ERR("WSAIoctl SIO_GET_EXTENSION_FUNCTION_POINTER");
        return NULL;
    }

    return lpfnWSARecvMsg;

}


LPFN_WSASENDMSG GetWSASendMsgFunctionPointer()
{
    LPFN_WSASENDMSG     lpfnWSASendMsg = NULL;
    GUID                guidWSASendMsg = WSAID_WSASENDMSG;
    SOCKET              sock = INVALID_SOCKET;
    DWORD               dwBytes = 0;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(SOCKET_ERROR == WSAIoctl(sock, 
                                SIO_GET_EXTENSION_FUNCTION_POINTER, 
                                &guidWSASendMsg, 
                                sizeof(guidWSASendMsg), 
                                &lpfnWSASendMsg, 
                                sizeof(lpfnWSASendMsg), 
                                &dwBytes, 
                                NULL, 
                                NULL
                                ))
    {
        ERR("WSAIoctl SIO_GET_EXTENSION_FUNCTION_POINTER");
        CLOSESOCK(sock);
        return NULL;
    }

    CLOSESOCK(sock);
    return lpfnWSASendMsg;

}

int __cdecl main()
{
    WSADATA             wsd = {0};
    INT                 nErr = 0,
                        nStartup = 0,
                        nRet = 0;
    SOCKET              sock = INVALID_SOCKET;
    SOCKADDR_STORAGE    localaddr = {0},
                        destaddr = {0},
                        remoteaddr = {0};
    WSAOVERLAPPED       over = {0};
    WSABUF              wsabuf = {0};
    WSAMSG              wsamsg = {0};
    DWORD               dwBytes = 0,
                        dwFlags = 0;
    LPFN_WSARECVMSG     WSARecvMsg = NULL;
    LPFN_WSASENDMSG     WSASendMsg = NULL;

    __try
    {
        nErr = WSAStartup(WS_VER,&wsd);
        if (nErr)
        {
            WSASetLastError(nErr);
            ERR("WSAStartup");
            __leave;
        }
        else
            nStartup++;

        if (INVALID_SOCKET == (sock = socket(AF_INET6,SOCK_DGRAM,0)))
        {
            ERR("socket");
            __leave;
        }

        localaddr.ss_family = AF_INET6;

        SET_PORT((SOCKADDR*)&localaddr,DEFAULT_PORT);

        if (SOCKET_ERROR == bind(sock,(SOCKADDR*)&localaddr,sizeof localaddr))
        {
            ERR("bind");
            __leave;
        }

        // PktInfo

        if (!SetIpv6PktInfoOption(sock))
        {
            ERR("SetIpv6PktInfoOption");
            __leave;
        }

        if(!AllocAndInitIpv6PktInfo(&wsamsg))
        {
            ERR("AllocAndInitIpv6PktInfo");
            __leave;
        }

        // data buffer

        wsabuf.buf = (CHAR*)MALLOC(DATA_SIZE);

        if (NULL == wsabuf.buf)
        {
            ERR("HeapAlloc");
            __leave;
        }

        wsabuf.len = (ULONG)MSIZE(wsabuf.buf);
        wsamsg.lpBuffers = &wsabuf;
        wsamsg.dwBufferCount = 1;

        // source address of packet

        wsamsg.name = (SOCKADDR*)&remoteaddr;
        wsamsg.namelen = sizeof remoteaddr;

        //Post overlapped RcvMsg

        InitOverlap(&over);

        if (NULL == (WSARecvMsg = GetWSARecvMsgFunctionPointer()))
        {
            ERR("GetWSARecvMsgFunctionPointer");
            __leave;
        }

        if (SOCKET_ERROR == WSARecvMsg(sock,
                                       &wsamsg,
                                       &dwBytes,
                                       &over,
                                       NULL
                                      ))
        {
            if (WSA_IO_PENDING != WSAGetLastError())
            {
                ERR("WSARecvMsg");
                __leave;
            }
        }

        //set destination address

        destaddr.ss_family = AF_INET6;

        INETADDR_SETLOOPBACK((SOCKADDR*)&destaddr);

        SET_PORT((SOCKADDR*)&destaddr,DEFAULT_PORT);

        //send some data

        if (SOCKET_ERROR == (nRet = sendto(sock,
                                           TST_MSG,
                                           lstrlen(TST_MSG),
                                           0,
                                           (SOCKADDR*)&destaddr,
                                           sizeof destaddr
                                           )))
        {
            ERR("sendto");
            __leave;
        }

        //check RecvMsg result

        if (SOCKET_ERROR == WSAGetOverlappedResult(sock,&over,&dwBytes,TRUE,&dwFlags))
        {
            ERR("WSAGetOverlappedResult");
            __leave;
        }

        printf("WSARecvMsg %d bytes\n",dwBytes);

        //Display address 

        if(SOCKET_ERROR == ProcessIpv6Msg(&wsamsg))
        {
            printf("Coult not process IPv6 message.\n");
        }
        
        //SendMsg with WSAMSG returned from RecvMsg

        
        if (NULL == (WSASendMsg = GetWSASendMsgFunctionPointer()))
        {
            ERR("GetWSARecvMsgFunctionPointer");
            __leave;
        }
        
        if (SOCKET_ERROR == (nRet = WSASendMsg(sock,
                                               &wsamsg,
                                               0,
                                               &dwBytes,
                                               NULL,
                                               NULL
                                              )))
        {
            ERR("WSASendMsg");
            __leave;
        }


        printf("WSASendMsg %d bytes\n",dwBytes);

        //RecvMsg to consume SendMsg data

        if (SOCKET_ERROR == WSARecvMsg(sock,
                                       &wsamsg,
                                       &dwBytes,
                                       NULL,
                                       NULL
                                      ))
        {
            ERR("WSARecvMsg");
            __leave;
        }

        printf("WSARecvMsg %d bytes\n",dwBytes);   

    }
    __finally
    {
        CLOSESOCK(sock);
        FREE(wsabuf.buf);
        FREE(wsamsg.Control.buf);
        CLOSESOCKEVENT(over.hEvent);
        if(nStartup) WSACleanup();
    }

    return 0;
}
