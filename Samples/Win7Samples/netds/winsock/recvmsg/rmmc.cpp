/******************************************************************************\
* rmmc.cpp
*
* This IPv6 sample demonstrates the use of WSARecvMsg() and the IPV6_PKTINFO option
* to determine multicast reception on a datagram socket. 
*
* WSARecvMsg is new to Windows Sockets in Windows XP. 
*
* This sample requires that TCP/IP version 6 be installed on the system.
*
* A IPv6 datagram socket is created and bound and the IPV6_PKTINFO option is set. 
* The socket is joined to multicast group and an overlapped WSARecvMsg is posted. 
* MSG_MCAST flag is checked upon completion, upon where further
* processing can be made. 
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
#include <iphlpapi.h>
#include <stdio.h>

// "safe" macros

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

//constants

#define WS_VER 0x0202

#define DEFAULT_PORT 12345

#define DEFAULT_WAIT 5000

#define MCAST_V6    "ff1f::1"

#define TST_MSG "Hello\0"

//Helper functions

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

VOID InitOverlap(LPWSAOVERLAPPED pOver)
{
    CLOSESOCKEVENT(pOver->hEvent);

    ZeroMemory(pOver,sizeof(WSAOVERLAPPED));

    if (WSA_INVALID_EVENT == (pOver->hEvent = WSACreateEvent()))
    {
        ERR("WSACreateEvent");
    }
}

VOID InitMcastAddr(SOCKADDR* pmcaddr,int size)
{
    WSAStringToAddressA(MCAST_V6,
                        AF_INET6,
                        NULL,
                        pmcaddr,
                        &size
                        );

    ((SOCKADDR_IN6*)pmcaddr)->sin6_port = htons(DEFAULT_PORT);


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

    if(NULL == CtrlBuf)
    {
        ERR("HeapAlloc");
        return FALSE;
    }
    
    pWSAMsg->Control.buf = (CHAR*)CtrlBuf;
    pWSAMsg->Control.len = (ULONG)MSIZE(CtrlBuf);

    return TRUE;
}

BOOL ProcessIpv6Msg(LPWSAMSG pWSAMsg)
{
    PWSACMSGHDR pCtrlInfo = NULL;
    PIN6_PKTINFO pPktInfo = NULL;

    pCtrlInfo = WSA_CMSG_FIRSTHDR(pWSAMsg);

    if ((IPPROTO_IPV6 == pCtrlInfo->cmsg_level) &&
        (IPV6_PKTINFO == pCtrlInfo->cmsg_type))
    {
        pPktInfo = (PIN6_PKTINFO)WSA_CMSG_DATA(pCtrlInfo);

        //do something with pPktInfo

        return TRUE;

    }

    return FALSE;
}

BOOL RouteLookup(SOCKADDR   *destAddr,
                 int         destLen,
                 SOCKADDR   *localAddr,
                 int         localLen
                )
{
    DWORD       dwBytes = 0;
    BOOL        bRet = FALSE;
    CHAR        szAddr[MAX_PATH] = {0};
    SOCKET      s = INVALID_SOCKET;


    __try
    {

        if (INVALID_SOCKET == (s = socket(destAddr->sa_family,SOCK_DGRAM,0)))
        {
            ERR("socket");
            __leave;
        }

        if (SOCKET_ERROR == WSAIoctl(s,
                                     SIO_ROUTING_INTERFACE_QUERY,
                                     destAddr,
                                     destLen,
                                     localAddr,
                                     localLen,
                                     &dwBytes,
                                     NULL,
                                     NULL
                                    ))
        {
            ERR("WSAIoctl");
            __leave;
        }

        dwBytes = sizeof(szAddr);

        ZeroMemory(szAddr,dwBytes);

        WSAAddressToStringA(destAddr,
                            (DWORD)destLen,
                            NULL,
                            szAddr,
                            &dwBytes
                            );

        dwBytes = sizeof(szAddr);

        ZeroMemory(szAddr,dwBytes);

        WSAAddressToStringA(localAddr,
                            (DWORD)localLen,
                            NULL,
                            szAddr,
                            &dwBytes
                            );

        bRet = TRUE;
    }
    __finally
    {
        CLOSESOCK(s);
    }

    return bRet;
}


DWORD 
GetInterfaceIndexForAddress(
    SOCKADDR *pAddr
    )
{
    IP_ADAPTER_UNICAST_ADDRESS *pTmpUniAddr = NULL;
    IP_ADAPTER_ADDRESSES       *pAdaptAddr = NULL,
                               *pTmpAdaptAddr = NULL;
    MIB_IPADDRTABLE            *pMibTable = NULL;
    DWORD                       dwRet = 0,
                                dwReturn = (DWORD) SOCKET_ERROR,
                                dwSize = 0,
                                Family = AF_UNSPEC,
                                dwIndex = 0;
    BOOL                        bFound = FALSE;

    typedef ULONG (*LPFN_GETADAPTERSADDRESSES) (ULONG Family,DWORD Flags,PVOID Reserved,PIP_ADAPTER_ADDRESSES pAdapterAddresses,PULONG pOutBufLen);
    LPFN_GETADAPTERSADDRESSES fnGetAdaptersAddresses = (LPFN_GETADAPTERSADDRESSES)GetProcAddress(GetModuleHandleA("iphlpapi"),"GetAdaptersAddresses");

    __try
    {
        if (NULL == pAddr)
        {
            WSASetLastError(ERROR_INVALID_PARAMETER);
            ERR("GetInterfaceIndexForAddress: invalid parameter passed (pAddr == NULL)");
            __leave;
        }

        switch (pAddr->sa_family)
        {
            case AF_INET:
                Family = AF_INET;
                break;
            case AF_INET6:
                Family = AF_INET6;
                break;
            default:
                WSASetLastError(WSAEAFNOSUPPORT);
                __leave;
                break;
        }

        if (fnGetAdaptersAddresses)
        {


            if (ERROR_BUFFER_OVERFLOW != (dwRet = fnGetAdaptersAddresses(Family,
                            GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_DNS_SERVER,
                            NULL,
                            NULL,
                            &dwSize
                            )))
            {
                WSASetLastError(dwRet);
                ERR("GetAdaptersAddresses");
                __leave;
            }

            if (NULL == (pAdaptAddr = (IP_ADAPTER_ADDRESSES*)MALLOC(dwSize)))
            {
                ERR("HeapAlloc");
                __leave;
            }

            if (ERROR_SUCCESS != (dwRet = fnGetAdaptersAddresses(Family,
                            GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_DNS_SERVER,
                            NULL,
                            pAdaptAddr,
                            &dwSize
                            )))
            {
                WSASetLastError(dwRet);
                ERR("GetAdaptersAddresses");
                __leave;
            }

            //look at each IP_ADAPTER_ADDRESSES node 

            pTmpAdaptAddr = pAdaptAddr;

            while (pTmpAdaptAddr)
            {

                //look at each IP_ADAPTER_UNICAST_ADDRESS node

                pTmpUniAddr = pTmpAdaptAddr->FirstUnicastAddress;


                while (pTmpUniAddr)
                {



                    if (AF_INET == pTmpUniAddr->Address.lpSockaddr->sa_family)
                    {

                        if (IN4_ADDR_EQUAL(&((SOCKADDR_IN*)pAddr)->sin_addr,&((SOCKADDR_IN*)pTmpUniAddr->Address.lpSockaddr)->sin_addr))
                        {
                            dwReturn = pTmpAdaptAddr->IfIndex;
                            bFound = TRUE;

                            break;
                        }

                    }
                    else
                    {
                        if (IN6_ADDR_EQUAL(&((SOCKADDR_IN6*)pAddr)->sin6_addr, &((SOCKADDR_IN6*)pTmpUniAddr->Address.lpSockaddr)->sin6_addr ))
                        {
                            dwReturn = pTmpAdaptAddr->Ipv6IfIndex;
                            bFound = TRUE;

                            break;
                        }
                    }

                    pTmpUniAddr = pTmpUniAddr->Next;

                }

                if (bFound)
                    break;

                pTmpAdaptAddr = pTmpAdaptAddr->Next;

            }
        }  //end GetAdaptersAddresses
        else //use mibtable
        {
            //call with NULL to get size needed to alloc
            if (ERROR_INSUFFICIENT_BUFFER != (dwRet = GetIpAddrTable(NULL,
                            &dwSize,
                            TRUE
                            )))
            {
                WSASetLastError(dwRet);
                ERR("GetIpAddrTable");
                __leave;
            }

            if (NULL == (pMibTable = (MIB_IPADDRTABLE*)MALLOC(dwSize)))
            {
                ERR("HeapAlloc");
                __leave;
            }

            if (NO_ERROR != (dwRet = GetIpAddrTable(pMibTable,
                            &dwSize,
                            TRUE
                            )))
            {
                WSASetLastError(dwRet);
                ERR("GetIpAddrTable");
                __leave;
            }

            for (dwIndex = 0; dwIndex < pMibTable->dwNumEntries; dwIndex++)
            {
                if( ((SOCKADDR_IN*)pAddr)->sin_addr.s_addr == pMibTable->table[dwIndex].dwAddr)
                {
                    dwReturn = pMibTable->table[dwIndex].dwIndex;
                }
            }



        }//end use mibtable

    }
    __finally
    {
        FREE(pAdaptAddr);
        FREE(pMibTable);
    }

    return dwReturn;
}

int SetSendInterface(SOCKET s, SOCKADDR *iface)
{
    char                    *optval=NULL;
    int                     optlevel = 0,
                            option = 0,
                            optlen = 0,
                            rc = NO_ERROR;
    DWORD                   dwIPv6Index = 0;


    if (iface->sa_family == AF_INET)
    {
        // Setup the v4 option values
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_IF;
        optval   = (char *) &((SOCKADDR_IN *)iface)->sin_addr.s_addr;
        optlen   = sizeof(((SOCKADDR_IN *)iface)->sin_addr.s_addr);
    }
    else if (iface->sa_family == AF_INET6)
    {
        // Setup the v6 option values
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_IF;
        if (SOCKET_ERROR == (dwIPv6Index = GetInterfaceIndexForAddress(iface)))
        {
            rc = SOCKET_ERROR;
        }
        optval   = (char *) &dwIPv6Index;
        optlen   = sizeof(dwIPv6Index);
    }
    else
    {
        WSASetLastError(WSAEAFNOSUPPORT);
        rc = SOCKET_ERROR;
    }
    // Set send IF
    if (rc != SOCKET_ERROR)
    {
        // Set the send interface
        rc = setsockopt(
                       s,
                       optlevel,
                       option,
                       optval,
                       optlen
                       );
        if(SOCKET_ERROR == rc)
        {
            ERR("setsockopt");

        }

    }

    return rc;
}

LPFN_WSARECVMSG GetWSARecvMsgFunctionPointer()
{
    LPFN_WSARECVMSG     lpfnWSARecvMsg = NULL;
    GUID                guidWSARecvMsg = WSAID_WSARECVMSG;
    SOCKET              sock = INVALID_SOCKET;
    DWORD               dwBytes = 0;

    sock = socket(AF_INET6,SOCK_DGRAM,0);

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

    CLOSESOCK(sock);

    return lpfnWSARecvMsg;
}


int __cdecl main()
{
    WSADATA             wsd;
    INT                 i = 0,
                        nErr = 0,
                        nStartup = 0,
                        rc = 0;
    SOCKET              sock = INVALID_SOCKET;
    SOCKADDR_STORAGE    addr = {0},
                        mcaddr = {0},
                        remoteaddr = {0};
    WSAOVERLAPPED       over = {0};
    WSABUF              wsabuf = {0};
    DWORD               dwBytes = 0,
                        dwFlags = 0,
                        dwRet = 0;
    IPV6_MREQ           mreq = {0};
    WSAMSG              wsamsg = {0};
    LPFN_WSARECVMSG     WSARecvMsg = NULL;


    __try
    {
        //Initialize Winsock

        nErr = WSAStartup(WS_VER,&wsd);
        if (nErr)
        {
            WSASetLastError(nErr);
            ERR("WSAStartup");
            __leave;
        }
        else
            nStartup++;


        // bind socket and register multicast

        mcaddr.ss_family = AF_INET6;

        InitMcastAddr((SOCKADDR*)&mcaddr,sizeof mcaddr);

        if (INVALID_SOCKET == (sock = socket(AF_INET6,SOCK_DGRAM,0)))
        {
            ERR("socket");
            __leave;
        }

        if(!RouteLookup((SOCKADDR*)&mcaddr,
                        sizeof mcaddr,
                        (SOCKADDR*)&addr,
                        sizeof addr
                        ))
        {
            ERR("RouteLookup");
            __leave;
        }
        
        SET_PORT((SOCKADDR*)&addr,DEFAULT_PORT);

        if (SOCKET_ERROR == bind(sock,(SOCKADDR*)&addr,sizeof addr))
        {
            ERR("bind");
            __leave;
        }

        mreq.ipv6mr_multiaddr = ((SOCKADDR_IN6*)&mcaddr)->sin6_addr;

        if (SOCKET_ERROR == setsockopt(sock,
                                       IPPROTO_IPV6,
                                       IPV6_ADD_MEMBERSHIP,
                                       (char*)&mreq,
                                       sizeof mreq
                                      ))
        {
            ERR("setsockopt IPV6_ADD_MEMBRESHIP");
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

        wsabuf.buf = (CHAR*)MALLOC(100);

        if(NULL == wsabuf.buf)
        {
            ERR("HeapAlloc");
            __leave;
        }

        wsabuf.len = (ULONG)MSIZE(wsabuf.buf);
        wsamsg.lpBuffers = &wsabuf;
        wsamsg.dwBufferCount = 1;

        // packet source address
        wsamsg.name = (SOCKADDR*)&remoteaddr;
        wsamsg.namelen = sizeof remoteaddr;

        //Post overlapped WSARecvMsg
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

        //set send interface

        if (SOCKET_ERROR == SetSendInterface(sock,(SOCKADDR*)&addr))
        {
            ERR("SetSendInterface");
            __leave;
        }

        //send msg to multicast


        SET_PORT((SOCKADDR*)&mcaddr,DEFAULT_PORT);

        //send a few packets
        for (i=0;i<5;i++)
        {

            if (SOCKET_ERROR == (rc = sendto(sock,
                                             TST_MSG,
                                             lstrlenA(TST_MSG),
                                             0,
                                             (SOCKADDR*)&mcaddr,
                                             sizeof (mcaddr)
                                            )))
            {
                ERR("sendto");
                __leave;
            }

            printf("Sent %d bytes\n",rc);
        }

        dwRet = WaitForSingleObject(over.hEvent,DEFAULT_WAIT);

        if (dwRet)
        {
            printf("%s\n",gai_strerror(dwRet));
            __leave;
        }

        if (!WSAGetOverlappedResult(sock,
                                    &over,
                                    &dwBytes,
                                    TRUE,
                                    &dwFlags
                                    ))
        {
            ERR("WSAGetOverlappedResult");
            __leave;
        }

        printf("WSARecvMsg completed with %d bytes\n",dwBytes);
        
        // if multicast packet do further processing
        if (MSG_MCAST & wsamsg.dwFlags)
        {
            if (ProcessIpv6Msg(&wsamsg))
            {
                //do something more interesting here
                printf("Recvd multicast msg.\n");
            }

        }
        
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
