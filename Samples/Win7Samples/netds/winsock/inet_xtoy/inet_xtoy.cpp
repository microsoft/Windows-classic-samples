
/******************************************************************************\
* inet_xtoy.cpp
*
* This sample demonstrates the use of inet_ntop, inet_pton, InetNtopW, and InetPtonW
* Windows Sockets address/string conversion functions. 
*
* These functions are new to to Windows Sockets in Windows Vista.
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista).
*
* IPv6 loopback address is set, and address is converted between string and address
* using each of the winsock inet functions.
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
#include <stdio.h>
#include <stdlib.h>

// "Safe" macros

        
#define ERR(e) \
        { \
        printf("%s:%s failed: %d [%s@%ld]\n",__FUNCTION__,e,WSAGetLastError(),__FILE__,__LINE__); \
        }


// Constants

#define WS_VER 0x0202

// Helpers

VOID PrintSockaddr(PSOCKADDR pSockaddr,DWORD dwSize)
{
    WCHAR szString[NI_MAXHOST] = {0};
    DWORD dwStringSize = sizeof szString;

    if (SOCKET_ERROR == WSAAddressToStringW(pSockaddr,
                                            dwSize,
                                            NULL,
                                            szString,
                                            &dwStringSize
                                            ))
    {
        ERR("WSAAddressToString");
        return;
    }

    wprintf(L"%s\n",szString);

    return;

}

int __cdecl main()
{
    WSADATA             wsd;
    INT                 nStartup = 0;
    PVOID               pAddr = NULL;
    SOCKADDR_STORAGE    addr = {0};
    CHAR                szAddr[NI_MAXHOST] = {0};
    WCHAR               wszAddr[NI_MAXHOST] = {0};

    __try
    {
        nStartup = WSAStartup(WS_VER,&wsd);
        if (nStartup)
        {
            ERR("WSAStartup");
            __leave;
        } else
            nStartup++;

        addr.ss_family = AF_INET6;

        INETADDR_SETLOOPBACK((PSOCKADDR)&addr);

        //Convert address to ANSI string

        pAddr = (PVOID)&((PSOCKADDR_IN6)&addr)->sin6_addr;

        if (!inet_ntop((INT)addr.ss_family,
                       pAddr,
                       szAddr,
                       sizeof szAddr
                      ))
        {
            ERR("inet_ntop");
            __leave;
        }

        printf("inet_ntop returned %s\n",szAddr);

        //Convert ANSI string to address

        if (!inet_pton((INT)addr.ss_family,
                       szAddr,
                       pAddr
                      ))
        {
            ERR("inet_pton");
            __leave;
        }

        printf("inet_pton returned ");
        PrintSockaddr((PSOCKADDR)&addr,sizeof addr);

        //Convert address to WIDE string

        if (!InetNtopW((INT)addr.ss_family,
                       pAddr,
                       wszAddr,
                       sizeof wszAddr
                      ))
        {
            ERR("InetNtopW");
            __leave;
        }

        wprintf(L"InetNtopW returned %s\n",wszAddr);

        //Convert WIDE string to address

        if (!InetPtonW((INT)addr.ss_family,
                       wszAddr,
                       pAddr
                      ))
        {
            ERR("InetPtonW");
            __leave;
        }

        printf("InetPtonW returned ");
        PrintSockaddr((PSOCKADDR)&addr,sizeof addr);

    }
    __finally
    {
        if (nStartup)
            WSACleanup();
    }

    return 0;
}


