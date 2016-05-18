// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Copyright (C) 1999 - 2000  Microsoft Corporation.  All Rights Reserved.
//
// Module:
//    rcvall.c
//
// Abstract:
//    This sample shows how to use the ioctls SIO_RCVALL,
//    SIO_RCVALL_MCAST, and SIO_RCVALL_IGMPMCAST. This sample
//    captures all packets of the given type and also is able
//    to set filters on source and destination IP addresses
//    and ports. This sample is Windows 2000 only.
//
// Usage:
//    rcvall.exe -t ip|igmp|multicast -i address -sa address -sp port
//               -da address -dp port
//           -t string           Filter traffic type
//                 ip            Capture all IP packets
//                 igmp          Capture all IGMP packets only
//                 multicast     Capture all multicast IP packets
//                 socket        Capter at the socket level only
//           -i address          Capture on this interface
//           -sa address         Filter on source address
//           -sp port            Filter on source port
//           -da address         Filter on dest address
//           -dp port            Filter on dest port
//
// Build:
//    cl rcvall.c parser.c resolve.c ws2_32.lib
// 
//       OR
//     
//    nmake.exe
//

#ifdef _IA64_
    #pragma warning(disable:4127 4706)
#endif 

#include <winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>

#include "parser.h"
#include "resolve.h"

#include <stdio.h>
#include <stdlib.h>

//
// Global Variables
//
SOCKADDR_STORAGE g_saLocalInterface = {0};
DWORD            g_dwIoControlCode=SIO_RCVALL,
                 g_dwProtocol=IPPROTO_IP,
                 g_dwIoControlValue=RCVALL_ON;
int              g_iAddressFamily=AF_UNSPEC;

int              g_iFamilyMap[] = {AF_INET, AF_INET6};


//
// Filter Global Variables
//
SOCKADDR_STORAGE g_saSourceAddress = {0},         // Source address to filter
                 g_saDestinationAddress = {0};    // Destination address to filter
unsigned long    g_ulFilterMask=0;          // Indicates which fields in IP hdr to
                                            //    filter on.

//
// Function Prototypes
//
void usage(char *progname);
int  ValidateArgs(int argc, char **argv);
void PrintInterfaceList();

//
// Function: usage
// 
// Description:
//    Prints usage information.
//
void usage(char *progname)
{
    printf("usage: %s -t traffic-type -i address ...\n\n", progname);
    printf("       -t string           Filter traffic type\n");
    printf("             Available traffic types:\n");
    printf("               ip          Capture all IP packets\n");
    printf("               igmp        Capture all IGMP packets only\n");
    printf("               multicast   Capture all multicast IP packets\n");
    printf("               socket      Capture at the socket level only\n");
    printf("       -i address          Capture on this interface\n");
    printf("             Available interfaces:\n");
    PrintInterfaceList();
    printf("       -sa address         Filter on source address\n");
    printf("       -sp Port            Filter on source port\n");
    printf("       -da address         Filter on dest address\n");
    printf("       -dp Port            Filter on dest port\n");

    WSACleanup();
    ExitProcess((UINT)-1);
}

//
// Function: ValidateArgs
// 
// Description:
//    This function parses the command line arguments and
//    sets global variables to indicate how the app should act.
//
int ValidateArgs(int argc, char **argv)
{
    struct addrinfo *resif=NULL,
                    *ressrc=NULL,
                    *resdest=NULL;
    char            *lpInterface=NULL,
                    *lpSourceAddress=NULL,
                    *lpDestinationAddress=NULL,
                    *lpSourcePort=NULL,
                    *lpDestinationPort=NULL;
    int    rc = 0, i = 0;

    rc = NO_ERROR;
    memset(&g_saLocalInterface, 0, sizeof(g_saLocalInterface));
    memset(&g_saSourceAddress,  0, sizeof(g_saSourceAddress));
    memset(&g_saDestinationAddress, 0, sizeof(g_saDestinationAddress));

    //
    // Parse the command line
    //
    for (i=1; i < argc ;i++)
    {
        if ((strlen(argv[i]) >= 2) && ((argv[i][0] == '-') || (argv[i][0] == '/')))
        {
            switch (tolower(argv[i][1]))
            {
            case 't':        // traffic type
                if (i+1 >= argc)
                    return SOCKET_ERROR;

                if ( _strnicmp(argv[i+1], "ip", 2) == 0 )
                {
                    g_dwIoControlCode = SIO_RCVALL;
                    g_dwIoControlValue= RCVALL_ON;
                } 
                else if ( _strnicmp(argv[i+1], "igmp", 4) == 0 )
                {
                    g_dwIoControlCode = SIO_RCVALL_IGMPMCAST;
                    g_dwProtocol = IPPROTO_IGMP;
                    g_dwIoControlValue= RCVALL_ON;
                } 
                else if ( _strnicmp(argv[i+1], "multicast", 9) == 0 )
                {
                    g_dwIoControlCode = SIO_RCVALL_MCAST;
                    g_dwIoControlValue= RCVALL_ON;
                } 
                else if ( _strnicmp(argv[i+1], "socket", 6) == 0 )
                {
                    g_dwIoControlCode = SIO_RCVALL;
                    g_dwIoControlValue= RCVALL_SOCKETLEVELONLY;
                }
                else
                {
                    return SOCKET_ERROR;
                }
                i++;
                break;
            case 'i':        // interface number
                lpInterface = argv[++i];
                break;
            case 's':        // Filter on source ip or port
                if ((i+1 >= argc) || (strlen(argv[i]) != 3))
                    return SOCKET_ERROR;

                if ( tolower(argv[i][2]) == 'a' )
                {
                    lpSourceAddress = argv[++i];
                    g_ulFilterMask |= FILTER_MASK_SOURCE_ADDRESS;
                } else if ( tolower(argv[i][2]) == 'p' )
                {
                    lpSourcePort = argv[++i];
                    g_ulFilterMask |= FILTER_MASK_SOURCE_PORT;
                } else
                {
                    return SOCKET_ERROR;
                }
                break;
            case 'd':        // Filter on dest ip or port
                if ((i+1 >= argc) || (strlen(argv[i]) != 3))
                    return SOCKET_ERROR;

                if ( tolower(argv[i][2]) == 'a' )
                {
                    lpDestinationAddress = argv[++i];
                    g_ulFilterMask |= FILTER_MASK_DESTINATION_ADDRESS;
                } else if ( tolower(argv[i][2]) == 'p' )
                {
                    lpDestinationPort = argv[++i];
                    g_ulFilterMask |= FILTER_MASK_DESTINATION_PORT;
                } else
                {
                    return SOCKET_ERROR;
                }
                break;
            default:
                return SOCKET_ERROR;
            }
        }
    }

    // Validate required arguments
    if (lpInterface == NULL)
    {
        fprintf(stderr, "\nA local interface must be specified!\n\n");
        return SOCKET_ERROR;
    }

    // Attempt to resolve the local interface
    resif = ResolveAddress(lpInterface, "0", g_iAddressFamily, 0, 0);
    if (resif == NULL)
    {
        fprintf(stderr, "\nUnable to resolve address %s, error = %d\n\n",
                lpInterface, WSAGetLastError());
        return SOCKET_ERROR;
    }

    // For SIO_RCVALL we may need to adjust the protocol value
    if ((g_dwIoControlCode == SIO_RCVALL) || (g_dwIoControlCode == SIO_RCVALL_MCAST))
    {
        if (resif->ai_family == AF_INET)
            g_dwProtocol = IPPROTO_IP;
        else if (resif->ai_family == AF_INET6)
            g_dwProtocol = IPPROTO_IPV6;
    }

    // Copy the interface address
    memcpy(&g_saLocalInterface, resif->ai_addr, resif->ai_addrlen);

    // Resolve the source address to filter on, if supplied
    if ((lpSourceAddress) || (lpSourcePort))
    {
        ressrc = ResolveAddress(lpSourceAddress, lpSourcePort, resif->ai_family, 0, 0);
        if (ressrc == NULL)
        {
            fprintf(stderr, "\nUnable to resolve source filter: %d\n\n", 
                    WSAGetLastError());
            rc = SOCKET_ERROR;
        } else
        {
            // Copy the source address filter
            memcpy(&g_saSourceAddress, ressrc->ai_addr, ressrc->ai_addrlen);

            freeaddrinfo(ressrc);
        }
    }

    // Resolve the destination address to filter on, if supplied
    if ((lpDestinationAddress) || (lpDestinationPort))
    {
        resdest = ResolveAddress(lpDestinationAddress, lpDestinationPort, resif->ai_family, 0, 0);
        if (resdest == NULL)
        {
            fprintf(stderr, "\nUnable to resolve destination filter: %d\n\n",
                    WSAGetLastError());
            rc = SOCKET_ERROR;
        } else
        {
            // Copy the destination address filter
            memcpy(&g_saDestinationAddress, resdest->ai_addr, resdest->ai_addrlen);

            freeaddrinfo(resdest);
        }
    }

    freeaddrinfo(resif);

    return rc;
}

//
// Function: PrintInterfaceList
//
// Description:
//    This function prints all local IP interfaces.
//
void PrintInterfaceList()
{
    SOCKET_ADDRESS_LIST *slist=NULL;
    SOCKET               s = INVALID_SOCKET;
    char                *buf=NULL;
    DWORD                dwBytesRet = 0;
    int                  rc = 0,
    i  = 0, j = 0, k = 0;

    k = 0;
    for (i=0; i < sizeof(g_iFamilyMap)/sizeof(int) ;i++)
    {
        s = socket(g_iFamilyMap[i], SOCK_STREAM, 0);
        if (s != INVALID_SOCKET)
        {
            rc = WSAIoctl(
                         s, 
                         SIO_ADDRESS_LIST_QUERY, 
                         NULL, 
                         0, 
                         NULL, 
                         0,
                         &dwBytesRet, 
                         NULL, 
                         NULL
                         );
            if ((rc == SOCKET_ERROR) && (GetLastError() == WSAEFAULT))
            {
                char addrbuf[INET6_ADDRSTRLEN] = {'\0'};

                // Allocate the necessary size
                buf = (char *)HeapAlloc(GetProcessHeap(), 0, dwBytesRet);
                if (buf == NULL)
                {
                    if(INVALID_SOCKET != s)
                    {
                        closesocket(s);
                        s = INVALID_SOCKET;
                    }
                    
                    return;
                }

                rc = WSAIoctl(
                             s, 
                             SIO_ADDRESS_LIST_QUERY, 
                             NULL, 
                             0, 
                             buf, 
                             dwBytesRet, 
                             &dwBytesRet, 
                             NULL, 
                             NULL
                             );
                if (rc == SOCKET_ERROR)
                {
                    if (buf) HeapFree(GetProcessHeap(), 0, buf);
                    if (INVALID_SOCKET != s)
                    {
                        closesocket(s);
                        s = INVALID_SOCKET;
                    }
                    return;
                }

                // Display the addresses
                slist = (SOCKET_ADDRESS_LIST *)buf;
                for (j=0; j < slist->iAddressCount ;j++)
                {
                    FormatAddress(
                                 slist->Address[j].lpSockaddr,
                                 slist->Address[j].iSockaddrLength,
                                 addrbuf,
                                 INET6_ADDRSTRLEN
                                 );
                    printf("               %-2d ........ %s\n", 
                           ++k, addrbuf);
                }

                if(buf) HeapFree(GetProcessHeap(), 0, buf);

            } else
            {
                // Unexpected failure
                fprintf(stderr, "WSAIoctl: SIO_ADDRESS_LIST_QUERY failed with unexpected error: %d\n",
                        WSAGetLastError());
            }
            if (INVALID_SOCKET != s)
            {
                closesocket(s);
                s = INVALID_SOCKET;
            }
        } else
        {
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            return;
        }
    }
    return;
}

#pragma warning(push)
#pragma warning(disable: 4127)

//
// Function: main
//
// Description:
//    This function loads Winsock, parses the command line, and
//    begins receiving packets. Once a packet is received they
//    are decoded. Because we are receiving IP datagrams, the
//    receive call will return whole datagrams.
//
int __cdecl main(int argc, char **argv)
{
    SOCKET        s = INVALID_SOCKET;
    WSADATA       wsd;
    WSABUF        wbuf = {0};
    DWORD         dwBytesRet = 0, dwFlags = 0;
    char         *rcvbuf=NULL;
    int           rc = 0, err;

    //
    // Load Winsock
    //
    if ((rc = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        printf("WSAStartup() failed: %d\n", rc);
        return -1;
    }

    //
    // Parse the command line
    //
    if (ValidateArgs(argc, argv) == SOCKET_ERROR)
    {
        usage(argv[0]);
    }

    if ( g_ulFilterMask & (FILTER_MASK_SOURCE_ADDRESS | FILTER_MASK_SOURCE_PORT) )
    {
        printf("Source address filter     : ");
        PrintAddress((SOCKADDR *)&g_saSourceAddress, sizeof(g_saSourceAddress));
        printf("\n");
    }
    if ( g_ulFilterMask & (FILTER_MASK_DESTINATION_ADDRESS | FILTER_MASK_DESTINATION_PORT) )
    {
        printf("Destination address filter: ");
        PrintAddress((SOCKADDR *)&g_saDestinationAddress, sizeof(g_saDestinationAddress));
        printf("\n");
    }

    //
    // Create a raw socket for receiving IP datagrams
    //
    s = WSASocket(g_saLocalInterface.ss_family, SOCK_RAW, g_dwProtocol, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
    {
        printf("WSASocket() failed: %d\n", WSAGetLastError());
        return -1;
    }

    //
    // This socket MUST be bound before calling the ioctl
    //
    rc = bind(s, (SOCKADDR *)&g_saLocalInterface, sizeof(g_saLocalInterface));
    if (rc == SOCKET_ERROR)
    {
        printf("bind() failed: %d\n", WSAGetLastError());
        if (INVALID_SOCKET != s)
        {
            closesocket(s);
            s = INVALID_SOCKET;
        }
        WSACleanup();
        return -1;
    }
    printf("Binding to: ");
    PrintAddress((SOCKADDR *)&g_saLocalInterface, sizeof(g_saLocalInterface));
    printf("\n");

    //
    // Set the SIO_RCVALLxxx ioctl
    //
    rc = WSAIoctl(s, g_dwIoControlCode, &g_dwIoControlValue, sizeof(g_dwIoControlValue),
                  NULL, 0, &dwBytesRet, NULL, NULL);
    if (rc == SOCKET_ERROR)
    {
        printf("WSAIotcl(0x%x) failed: %d\n", g_dwIoControlCode,
               (err = WSAGetLastError()));
        if (err == WSAEINVAL)
        {
            printf("NOTE: IPv6 does not currently support the SIO_RCVALL* ioctls\n");
        }

        if (INVALID_SOCKET != s)
        {
            closesocket(s);
            s = INVALID_SOCKET;
        }
        WSACleanup();
        return -1;
    }

    //
    // Allocate a buffer for receiving data
    //
    rcvbuf = (char *)HeapAlloc(GetProcessHeap(), 0, MAX_IP_SIZE);
    if (rcvbuf == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        if (INVALID_SOCKET != s)
        {
            closesocket(s);
            s = INVALID_SOCKET;
        }
        WSACleanup();
        return -1;
    }

    //
    // Start receiving IP datagrams until interrupted
    // 
    while (1)
    {
        wbuf.len = MAX_IP_SIZE;
        wbuf.buf = rcvbuf;
        dwFlags  = 0;

        rc = WSARecv(s, &wbuf, 1, &dwBytesRet, &dwFlags, NULL, NULL);
        if (rc == SOCKET_ERROR)
        {
            printf("WSARecv() failed: %d\n", WSAGetLastError());
            break;
        }
        // Decode the IP header
        //
        rc = DecodeIPHeader(
                           rcvbuf,
                           dwBytesRet, 
                           g_ulFilterMask,
                           (SOCKADDR *)&g_saSourceAddress,
                           (SOCKADDR *)&g_saDestinationAddress
                           );
        if (rc != NO_ERROR)
        {
            printf("Error decoding IP header!\n");
            break;
        }
    }
    //
    // Cleanup
    //

    if (rcvbuf) HeapFree(GetProcessHeap(), 0, rcvbuf);

    if (INVALID_SOCKET != s)
    {
        closesocket(s);
        s = INVALID_SOCKET;
    }
    WSACleanup();

    return 0;
}

#pragma warning(pop)
