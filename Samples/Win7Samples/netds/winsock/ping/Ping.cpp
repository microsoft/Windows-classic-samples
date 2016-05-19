// From Network Programming for Microsoft Windows, Second Edition by 
// Anthony Jones and James Ohlund.  
// Copyright 2002.   Reproduced by permission of Microsoft Press.  
// All rights reserved.
//
//
// Sample: IPv4 and IPv6 Ping Sample
//
// Files:
//    iphdr.h       - IPv4 and IPv6 packet header definitions
//    ping.cpp      - this file
//    resolve.cpp   - Common name resolution routine
//    resolve.h     - Header file for common name resolution routines
//
// Description:
//    This sample illustrates how to use raw sockets to send ICMP
//    echo requests and receive their response. This sample performs
//    both IPv4 and IPv6 ICMP echo requests. When using raw sockets,
//    the protocol value supplied to the socket API is used as the
//    protocol field (or next header field) of the IP packet. Then
//    as a part of the data submitted to sendto, we include both
//    the ICMP request and data.
//
//    For IPv4 the IP record route option is supported via the 
//    IP_OPTIONS socket option.
//
// Compile:
//      cl -o ping.exe ping.cpp resolve.cpp ws2_32.lib
//
// Command Line Options/Parameters:
//     ping.exe [-a 4|6] [-i ttl] [-l datasize] [-r] [host]
//     
//     -a       Address family (IPv4 or IPv6)
//     -i ttl   TTL value to set on socket
//     -l size  Amount of data to send as part of the ICMP request
//     -r       Use IPv4 record route
//     host     Hostname or literal address
//
#ifdef _IA64_
#pragma warning (disable: 4267)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include "resolve.h"

#include "iphdr.h"

#define DEFAULT_DATA_SIZE      32       // default data size

#define DEFAULT_SEND_COUNT     4        // number of ICMP requests to send

#define DEFAULT_RECV_TIMEOUT   6000     // six second

#define DEFAULT_TTL            128

#define MAX_RECV_BUF_LEN       0xFFFF   // Max incoming packet size.

int   gAddressFamily=AF_UNSPEC,         // Address family to use
      gProtocol=IPPROTO_ICMP,           // Protocol value
      gTtl=DEFAULT_TTL;                 // Default TTL value
int   gDataSize=DEFAULT_DATA_SIZE;      // Amount of data to send
BOOL  bRecordRoute=FALSE;               // Use IPv4 record route?
char *gDestination=NULL,                // Destination
      recvbuf[MAX_RECV_BUF_LEN];        // For received packets
int   recvbuflen = MAX_RECV_BUF_LEN;    // Length of received packets.


//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
    printf("usage: %s [options] <host> \n", progname);
    printf("        host        Remote machine to ping\n");
    printf("        options: \n");
    printf("            -a 4|6       Address family (default: AF_UNSPEC)\n");
    printf("            -i ttl       Time to live (default: 128) \n");
    printf("            -l bytes     Amount of data to send (default: 32) \n");
    printf("            -r           Record route (IPv4 only)\n");

    return;
}

// 
// Function: InitIcmpHeader
//
// Description:
//    Helper function to fill in various stuff in our ICMP request.
//
void InitIcmpHeader(char *buf, int datasize)
{
    ICMP_HDR   *icmp_hdr=NULL;
    char       *datapart=NULL;

    icmp_hdr = (ICMP_HDR *)buf;
    icmp_hdr->icmp_type     = ICMPV4_ECHO_REQUEST_TYPE;        // request an ICMP echo
    icmp_hdr->icmp_code     = ICMPV4_ECHO_REQUEST_CODE;
    icmp_hdr->icmp_id       = (USHORT)GetCurrentProcessId();
    icmp_hdr->icmp_checksum = 0;
    icmp_hdr->icmp_sequence = 0;
  
    datapart = buf + sizeof(ICMP_HDR);
    //
    // Place some data in the buffer.
    //
    memset(datapart, 'E', datasize);
}

//
// Function: InitIcmp6Header
//
// Description:
//    Initialize the ICMP6 header as well as the echo request header.
//
int InitIcmp6Header(char *buf, int datasize)
{
    ICMPV6_HDR          *icmp6_hdr=NULL;
    ICMPV6_ECHO_REQUEST *icmp6_req=NULL;
    char                *datapart=NULL;

    // Initialize the ICMP6 headerf ields
    icmp6_hdr = (ICMPV6_HDR *)buf;
    icmp6_hdr->icmp6_type     = ICMPV6_ECHO_REQUEST_TYPE;
    icmp6_hdr->icmp6_code     = ICMPV6_ECHO_REQUEST_CODE;
    icmp6_hdr->icmp6_checksum = 0;

    // Initialize the echo request fields
    icmp6_req = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));
    icmp6_req->icmp6_echo_id       = (USHORT)GetCurrentProcessId();
    icmp6_req->icmp6_echo_sequence = 0;

    datapart = (char *)buf + sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

    memset(datapart, '#', datasize);

    return (sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST));
}

// 
// Function: checksum
//
// Description:
//    This function calculates the 16-bit one's complement sum
//    of the supplied buffer (ICMP) header.
//
USHORT checksum(USHORT *buffer, int size) 
{
    unsigned long cksum=0;

    while (size > 1) 
    {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }
    if (size) 
    {
        cksum += *(UCHAR*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (USHORT)(~cksum);
}

//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments.
//
BOOL ValidateArgs(int argc, char **argv)
{
    int                i;
    BOOL               isValid = FALSE;

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (tolower(argv[i][1]))
            {
                case 'a':        // address family
                    if (i+1 >= argc)
                    {
                        usage(argv[0]);
                        goto CLEANUP;
                    }
                    if (argv[i+1][0] == '4')
                        gAddressFamily = AF_INET;
                    else if (argv[i+1][0] == '6')
                        gAddressFamily = AF_INET6;
                    else
                    {
                        usage(argv[0]);
                        goto CLEANUP;
                    }

                    i++;
                    break;
                case 'i':        // Set TTL value
                    if (i+1 >= argc)
                    {
                        usage(argv[0]);
                        goto CLEANUP;
                    }

                    gTtl = atoi(argv[++i]);
                    break;
                case 'l':        // buffer size tos end
                    if (i+1 >= argc)
                    {
                        usage(argv[0]);
                        goto CLEANUP;
                    }

                    gDataSize = atoi(argv[++i]);
                    break;
                case 'r':        // record route option
                    bRecordRoute = TRUE;
                    break;
                default:
                    usage(argv[0]);
                    goto CLEANUP;
            }
        }
        else
        {
            gDestination = argv[i];
        }
    }

    isValid = TRUE;

CLEANUP:    
    return isValid;
}

//
// Function: SetIcmpSequence
//
// Description:
//    This routine sets the sequence number of the ICMP request packet.
//
void SetIcmpSequence(char *buf)
{
    ULONG    sequence=0;

    sequence = GetTickCount();
    if (gAddressFamily == AF_INET)
    {
        ICMP_HDR    *icmpv4=NULL;

        icmpv4 = (ICMP_HDR *)buf;

        icmpv4->icmp_sequence = (USHORT)sequence;
    }
    else if (gAddressFamily == AF_INET6)
    {
        ICMPV6_HDR          *icmpv6=NULL;
        ICMPV6_ECHO_REQUEST *req6=NULL;

        icmpv6 = (ICMPV6_HDR *)buf;
        req6   = (ICMPV6_ECHO_REQUEST *)(buf + sizeof(ICMPV6_HDR));

        req6->icmp6_echo_sequence = (USHORT)sequence;
    }
}

//
// Function: ComputeIcmp6PseudoHeaderChecksum
//
// Description:
//    This routine computes the ICMP6 checksum which includes the pseudo
//    header of the IPv6 header (see RFC2460 and RFC2463). The one difficulty
//    here is we have to know the source and destination IPv6 addresses which
//    will be contained in the IPv6 header in order to compute the checksum.
//    To do this we call the SIO_ROUTING_INTERFACE_QUERY ioctl to find which
//    local interface for the outgoing packet.
//
USHORT ComputeIcmp6PseudoHeaderChecksum(SOCKET s, char *icmppacket, int icmplen, struct addrinfo *dest)
{
    SOCKADDR_STORAGE localif;
    DWORD            bytes;
    char             tmp[MAX_RECV_BUF_LEN] = {'\0'},
                    *ptr=NULL,
                     proto=0;
    int              rc, total, length, i;

    // Find out which local interface for the destination
    rc = WSAIoctl(
            s,
            SIO_ROUTING_INTERFACE_QUERY,
            dest->ai_addr,
            (DWORD) dest->ai_addrlen,
            (SOCKADDR *) &localif,
            (DWORD) sizeof(localif),
           &bytes,
            NULL,
            NULL
            );
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "WSAIoctl failed: %d\n", WSAGetLastError());
        return 0xFFFF;
    }

    // We use a temporary buffer to calculate the pseudo header. 
    ptr = tmp;
    total = 0;

    // Copy source address
    memcpy(ptr, &((SOCKADDR_IN6 *)&localif)->sin6_addr, sizeof(struct in6_addr));
    ptr   += sizeof(struct in6_addr);
    total += sizeof(struct in6_addr);

    printf("%x%x%x%x\n", 
            ((SOCKADDR_IN6 *) &localif)->sin6_addr.u.Byte[0],
            ((SOCKADDR_IN6 *) &localif)->sin6_addr.u.Byte[1],
            ((SOCKADDR_IN6 *) &localif)->sin6_addr.u.Byte[2],
            ((SOCKADDR_IN6 *) &localif)->sin6_addr.u.Byte[3]
            );

    // Copy destination address
    memcpy(ptr, &((SOCKADDR_IN6 *)dest->ai_addr)->sin6_addr, sizeof(struct in6_addr));
    ptr   += sizeof(struct in6_addr);
    total += sizeof(struct in6_addr);

    // Copy ICMP packet length
    length = htonl(icmplen);

    memcpy(ptr, &length, sizeof(length));
    ptr   += sizeof(length);
    total += sizeof(length);

    printf("%x%x%x%x\n", 
            (char ) *(ptr - 4),
            (char ) *(ptr - 3),
            (char ) *(ptr - 2),
            (char ) *(ptr - 1)
            );

    // Zero the 3 bytes
    memset(ptr, 0, 3);
    ptr   += 3;
    total += 3;

    // Copy next hop header
    proto = IPPROTO_ICMP6;

    memcpy(ptr, &proto, sizeof(proto));
    ptr   += sizeof(proto);
    total += sizeof(proto);

    // Copy the ICMP header and payload
    memcpy(ptr, icmppacket, icmplen);
    ptr   += icmplen;
    total += icmplen;

    for(i=0; i < icmplen%2 ;i++)
    {
        *ptr = 0;
        ptr++;
        total++;
    }

    
    return checksum((USHORT *)tmp, total);
}

//
// Function: ComputeIcmpChecksum
//
// Description:
//    This routine computes the checksum for the ICMP request. For IPv4 its
//    easy, just compute the checksum for the ICMP packet and data. For IPv6,
//    its more complicated. The pseudo checksum has to be computed for IPv6
//    which includes the ICMP6 packet and data plus portions of the IPv6
//    header which is difficult since we aren't building our own IPv6
//    header.
//
void ComputeIcmpChecksum(SOCKET s, char *buf, int packetlen, struct addrinfo *dest)
{
    if (gAddressFamily == AF_INET)
    {
        ICMP_HDR    *icmpv4=NULL;

        icmpv4 = (ICMP_HDR *)buf;
        icmpv4->icmp_checksum = 0;
        icmpv4->icmp_checksum = checksum((USHORT *)buf, packetlen);
    }
    else if (gAddressFamily == AF_INET6)
    {
        ICMPV6_HDR  *icmpv6=NULL;

        icmpv6 = (ICMPV6_HDR *)buf;
        icmpv6->icmp6_checksum = 0;
        icmpv6->icmp6_checksum = ComputeIcmp6PseudoHeaderChecksum(
                s,
                buf,
                packetlen,
                dest
                );
    }
}

//
// Function: PostRecvfrom
//
// Description:
//    This routine posts an overlapped WSARecvFrom on the raw socket.
//
int PostRecvfrom(SOCKET s, char *buf, int buflen, SOCKADDR *from, int *fromlen, WSAOVERLAPPED *ol)
{
    WSABUF  wbuf;
    DWORD   flags,
            bytes;
    int     rc;

    wbuf.buf = buf;
    wbuf.len = buflen;

    flags = 0;

    rc = WSARecvFrom(
            s,
           &wbuf,
            1,
           &bytes,
           &flags,
            from,
            fromlen,
            ol,
            NULL
            );
    if (rc == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            fprintf(stderr, "WSARecvFrom failed: %d\n", WSAGetLastError());
            return SOCKET_ERROR;
        }
    }
    return NO_ERROR;
}

//
// Function: PrintPayload
// 
// Description:
//    This routine is for IPv4 only. It determines if there are any IP options
//    present (by seeing if the IP header length is greater than 20 bytes) and
//    if so it prints the IP record route options.
//
void PrintPayload(char *buf, int bytes)
{
    int     hdrlen=0,
            routes=0,
            i;

    UNREFERENCED_PARAMETER(bytes);

    if (gAddressFamily == AF_INET)
    {
        SOCKADDR_IN      hop;
        IPV4_OPTION_HDR *v4opt=NULL;
        IPV4_HDR        *v4hdr=NULL;

        hop.sin_family = (USHORT)gAddressFamily;
        hop.sin_port   = 0;

        v4hdr = (IPV4_HDR *)buf;
        hdrlen = (v4hdr->ip_verlen & 0x0F) * 4;

        // If the header length is greater than the size of the basic IPv4
        //    header then there are options present. Find them and print them.
        if (hdrlen > sizeof(IPV4_HDR))
        {
            v4opt = (IPV4_OPTION_HDR *)(buf + sizeof(IPV4_HDR));
            routes = (v4opt->opt_ptr / sizeof(ULONG)) - 1;
            for(i=0; i < routes ;i++)
            {
                hop.sin_addr.s_addr = v4opt->opt_addr[i];

                // Print the route
                if (i == 0)
                    printf("    Route: ");
                else
                    printf("           ");
                PrintAddress((SOCKADDR *)&hop, sizeof(hop));

                if (i < routes-1)
                    printf(" ->\n");
                else
                    printf("\n");
            }
        }
    }
    return;
}

//
// Function: SetTtl
//
// Description:
//    Sets the TTL on the socket.
//
int SetTtl(SOCKET s, int ttl)
{
    int     optlevel = 0,
            option = 0,
            rc;

    rc = NO_ERROR;
    if (gAddressFamily == AF_INET)
    {
        optlevel = IPPROTO_IP;
        option   = IP_TTL;
    }
    else if (gAddressFamily == AF_INET6)
    {
        optlevel = IPPROTO_IPV6;
        option   = IPV6_UNICAST_HOPS;
    }
    else
    {
        rc = SOCKET_ERROR;
    }
    if (rc == NO_ERROR)
    {
        rc = setsockopt(
                s,
                optlevel,
                option,
                (char *)&ttl,
                sizeof(ttl)
                );
    }
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "SetTtl: setsockopt failed: %d\n", WSAGetLastError());
    }
    return rc;
}
        
//
// Function: main
//
// Description:
//    Setup the ICMP raw socket and create the ICMP header. Add
//    the appropriate IP option header and start sending ICMP
//    echo requests to the endpoint. For each send and receive we
//    set a timeout value so that we don't wait forever for a 
//    response in case the endpoint is not responding. When we
//    receive a packet decode it.
//

int __cdecl main(int argc, char **argv)
{

    WSADATA            wsd;
    WSAOVERLAPPED      recvol;
    SOCKET             s=INVALID_SOCKET;
    char              *icmpbuf=NULL;
    struct addrinfo   *dest=NULL,
                      *local=NULL;
    IPV4_OPTION_HDR    ipopt;
    SOCKADDR_STORAGE   from;
    DWORD              bytes,
                       flags;
    int                packetlen=0,
                       fromlen,
                       time=0,
                       rc,
                       i,
                       status = 0;

    recvol.hEvent = WSA_INVALID_EVENT;

    // Parse the command line
    if (ValidateArgs(argc, argv) == FALSE)
    {
        // invalid arguments supplied.
        status = -1;
        goto EXIT;
    }

    // Load Winsock
    if ((rc = WSAStartup(MAKEWORD(2,2), &wsd)) != 0)
    {
        printf("WSAStartup() failed: %d\n", rc);
        status = -1;
        goto EXIT;
    }

    // Resolve the destination address
    dest = ResolveAddress(
            gDestination,
            "0",
            gAddressFamily,
            0,
            0
            );
    if (dest == NULL)
    {
        printf("bad name %s\n", gDestination);
        status = -1;
        goto CLEANUP;
    }
    gAddressFamily = dest->ai_family;

    if (gAddressFamily == AF_INET)
        gProtocol = IPPROTO_ICMP;
    else if (gAddressFamily == AF_INET6)
        gProtocol = IPPROTO_ICMP6;

    // Get the bind address
    local = ResolveAddress(
            NULL,
            "0",
            gAddressFamily,
            0,
            0
            );
    if (local == NULL)
    {
        printf("Unable to obtain the bind address!\n");
        status = -1;
        goto CLEANUP;
    }

    // Create the raw socket
    s = socket(gAddressFamily, SOCK_RAW, gProtocol);
    if (s == INVALID_SOCKET) 
    {
        printf("socket failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    SetTtl(s, gTtl);

    // Figure out the size of the ICMP header and payload
    if (gAddressFamily == AF_INET)
        packetlen += sizeof(ICMP_HDR);
    else if (gAddressFamily == AF_INET6)
        packetlen += sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

    // Add in the data size
    packetlen += gDataSize;

    // Allocate the buffer that will contain the ICMP request
    icmpbuf = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, packetlen);
    if (icmpbuf == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        status = -1;
        goto CLEANUP;
    }

    // Initialize the ICMP headers
    if (gAddressFamily == AF_INET)
    {
        if (bRecordRoute)
        {
            // Setup the IP option header to go out on every ICMP packet
            ZeroMemory(&ipopt, sizeof(ipopt));
            ipopt.opt_code = IP_RECORD_ROUTE; // record route option
            ipopt.opt_ptr  = 4;               // point to the first addr offset
            ipopt.opt_len  = 39;              // length of option header

            rc = setsockopt(s, IPPROTO_IP, IP_OPTIONS, 
                    (char *)&ipopt, sizeof(ipopt));
            if (rc == SOCKET_ERROR)
            {
                fprintf(stderr, "setsockopt(IP_OPTIONS) failed: %d\n", WSAGetLastError());
                status = -1;
                goto CLEANUP;
            }
        }

        InitIcmpHeader(icmpbuf, gDataSize);
    }
    else if (gAddressFamily == AF_INET6)
    {
        InitIcmp6Header(icmpbuf, gDataSize);
    }

    // Bind the socket -- need to do this since we post a receive first
    rc = bind(s, local->ai_addr, (int)local->ai_addrlen);
    if (rc == SOCKET_ERROR)
    {
        fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    // Setup the receive operation
    memset(&recvol, 0, sizeof(recvol));
    recvol.hEvent = WSACreateEvent();
    if (recvol.hEvent == WSA_INVALID_EVENT)
    {
        fprintf(stderr, "WSACreateEvent failed: %d\n", WSAGetLastError());
        status = -1;
        goto CLEANUP;
    }

    // Post the first overlapped receive
    fromlen = sizeof(from);
    PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR *)&from, &fromlen, &recvol);

    printf("\nPinging ");
    PrintAddress(dest->ai_addr, (int)dest->ai_addrlen);
    printf(" with %d bytes of data\n\n", gDataSize);

    // Start sending the ICMP requests
    for(i=0; i < DEFAULT_SEND_COUNT ;i++)
    {
        // Set the sequence number and compute the checksum
        SetIcmpSequence(icmpbuf);
        ComputeIcmpChecksum(s, icmpbuf, packetlen, dest);

        time = GetTickCount();
        rc = sendto(
                s,
                icmpbuf,
                packetlen,
                0,
                dest->ai_addr,
                (int)dest->ai_addrlen
                );
        if (rc == SOCKET_ERROR)
        {
            fprintf(stderr, "sendto failed: %d\n", WSAGetLastError());
            status = -1;
            goto CLEANUP;
        }

        // Waite for a response
        rc = WaitForSingleObject((HANDLE)recvol.hEvent, DEFAULT_RECV_TIMEOUT);
        if (rc == WAIT_FAILED)
        {
            fprintf(stderr, "WaitForSingleObject failed: %d\n", GetLastError());
            status = -1;
            goto CLEANUP;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            printf("Request timed out.\n");
        }
        else
        {
            rc = WSAGetOverlappedResult(
                   s,
                   &recvol,
                   &bytes,
                    FALSE,
                   &flags
                    );
            if (rc == FALSE)
            {
                fprintf(stderr, "WSAGetOverlappedResult failed: %d\n", WSAGetLastError());
            }
            time = GetTickCount() - time;

            WSAResetEvent(recvol.hEvent);

            printf("Reply from ");
            PrintAddress((SOCKADDR *)&from, fromlen);
            if (time == 0)
                printf(": bytes=%d time<1ms TTL=%d\n", gDataSize, gTtl);
            else
                printf(": bytes=%d time=%dms TTL=%d\n", gDataSize, time, gTtl);

            PrintPayload(recvbuf, bytes);

            if (i < DEFAULT_SEND_COUNT - 1)
            {
                fromlen = sizeof(from);
                PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR *)&from, &fromlen, &recvol);
            }
        }
        Sleep(1000);
    }

CLEANUP:
    
    //
    // Cleanup
    //
    if (dest)
         freeaddrinfo(dest);
    if (local)
         freeaddrinfo(local);
    if (s != INVALID_SOCKET) 
        closesocket(s);
    if (recvol.hEvent != WSA_INVALID_EVENT)
        WSACloseEvent(recvol.hEvent);
    if (icmpbuf)
        HeapFree(GetProcessHeap(), 0, icmpbuf);

    WSACleanup();

EXIT:
    return status;
}
