// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Copyright (C) 1999 - 2000  Microsoft Corporation.  All Rights Reserved.
//
// Module:
//    parser.c
//
// Abstract:
//    This sample illustrates how to use the new Winsock 2 ioctls:
//    SIO_RCVALL, SIO_RCVALL_MCAST, and SIO_RCVALL_IGMPMCAST.
//    
//    This module includes routines for parsing the IP headers read.
//    Not all protocol headers are parsed, just the most common ones
//    (IPv4, TCP, UDP, and IGMP). You can easily extend this to parse
//    any protocol headers not covered. Additionally, code for parsing
//    IPv6 headers is not provided as the SIO_RCVALL* ioctls are not
//    currently supported by the IPv6 stack.
// 
// Usage:
//    See rcvall.c
//
// Build:
//    See rcvall.c
//
//

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "resolve.h"
#include "iphdr.h"

//
// A list of protocol types in the IP protocol header
//
char *szProto[] = {"Reserved",     //  0
                   "ICMP",         //  1
                   "IGMP",         //  2
                   "GGP",          //  3
                   "IP",           //  4
                   "ST",           //  5
                   "TCP",          //  6
                   "UCL",          //  7
                   "EGP",          //  8
                   "IGP",          //  9
                   "BBN-RCC-MON",  // 10
                   "NVP-II",       // 11
                   "PUP",          // 12
                   "ARGUS",        // 13
                   "EMCON",        // 14
                   "XNET",         // 15
                   "CHAOS",        // 16
                   "UDP",          // 17
                   "MUX",          // 18
                   "DCN-MEAS",     // 19
                   "HMP",          // 20
                   "PRM",          // 21
                   "XNS-IDP",      // 22
                   "TRUNK-1",      // 23
                   "TRUNK-2",      // 24
                   "LEAF-1",       // 25
                   "LEAF-2",       // 26
                   "RDP",          // 27
                   "IRTP",         // 28
                   "ISO-TP4",      // 29
                   "NETBLT",       // 30
                   "MFE-NSP",      // 31
                   "MERIT-INP",    // 32
                   "SEP",          // 33
                   "3PC",          // 34
                   "IDPR",         // 35
                   "XTP",          // 36
                   "DDP",          // 37
                   "IDPR-CMTP",    // 38
                   "TP++",         // 39
                   "IL",           // 40
                   "SIP",          // 41
                   "SDRP",         // 42
                   "SIP-SR",       // 43
                   "SIP-FRAG",     // 44
                   "IDRP",         // 45
                   "RSVP",         // 46
                   "GRE",          // 47
                   "MHRP",         // 48
                   "BNA",          // 49
                   "SIPP-ESP",     // 50
                   "SIPP-AH",      // 51
                   "I-NLSP",       // 52
                   "SWIPE",        // 53
                   "NHRP",         // 54
                   "unassigned",   // 55
                   "unassigned",   // 56
                   "unassigned",   // 57
                   "unassigned",   // 58
                   "unassigned",   // 59
                   "unassigned",   // 60
                   "any host internal protocol",  // 61
                   "CFTP",         // 62
                   "any local network",           // 63
                   "SAT-EXPAK",    // 64
                   "KRYPTOLAN",    // 65
                   "RVD",          // 66
                   "IPPC",         // 67
                   "any distributed file system", // 68
                   "SAT-MON",    // 69
                   "VISA",       // 70
                   "IPCV",       // 71
                   "CPNX",       // 72
                   "CPHB",       // 73
                   "WSN",        // 74
                   "PVP",        // 75
                   "BR-SAT-MON", // 76
                   "SUN-ND",     // 77
                   "WB-MON",     // 78
                   "WB-EXPAK",   // 79
                   "ISO-IP",     // 80
                   "VMTP",       // 81
                   "SECURE-VMTP",// 82
                   "VINES",      // 83
                   "TTP",        // 84
                   "NSFNET-IGP", // 85
                   "DGP",        // 86
                   "TCF",        // 87
                   "IGRP",       // 88
                   "OSPFIGP",    // 89
                   "Sprite-RPC", // 90
                   "LARP",       // 91
                   "MTP",        // 92
                   "AX.25",      // 93
                   "IPIP",       // 94
                   "MICP",       // 95
                   "SCC-SP",     // 96
                   "ETHERIP",    // 97
                   "ENCAP",      // 98
                   "any private encryption scheme",    // 98
                   "GMTP"        // 99
                  };

//
// The types of IGMP messages
//
char *szIgmpType[] = {"Invalid - 0",
                      "Host Membership Query",
                      "Host Membership Report",
                      "Invalid - 3",
                      "Invalid - 4",
                      "Invalid - 5",
                      "Version 2 Membership Report",
                      "Leave Group"
                     };

//
// IGMPv3 Modes
//
char *szIgmpMode[] = {"Invalid-Mode-0",
                      "MODE_IS_INCLUDE",
                      "MODE_IS_EXCLUDE",
                      "CHANGE_TO_INCLUDE_MODE",
                      "CHANGE_TO_EXCLUDE_MODE",
                      "ALLOW_NEW_SOURCES",
                      "BLOCK_OLD_SOURCES"
                      "Invalid-Mode-7",
                      "Invalid-Mode-8"
                     };

//
// Function: PrintIpv4Header
//
// Description:
//    Prints the IPv4 header to the console in a readable form.
//
void
PrintIpv4Header(
    IPV4_HDR *v4hdr,
    int len
    )
{
    SOCKADDR_IN sa4 = {0};
    char        srcaddr[INET_ADDRSTRLEN] = {'\0'},
                destaddr[INET_ADDRSTRLEN] = {'\0'};

    UNREFERENCED_PARAMETER( len );

    printf("IPv4: Version         %2d : Length     %2d : Type of Service 0x%02x\n",
            HI_BYTE(v4hdr->ip_verlen), LO_BYTE(v4hdr->ip_verlen) * 4, v4hdr->ip_tos);
    printf("IPv4: Total Length %5d : ID     0x%04x : Time to Live    %d\n",
            ntohs(v4hdr->ip_totallength), ntohs(v4hdr->ip_offset), v4hdr->ip_ttl);
    // Make sure protocol value is in the array of szProto
    if ((v4hdr->ip_protocol >= 0) && (v4hdr->ip_protocol < (sizeof(szProto) / sizeof(char *))))
        printf("IPv4: Protocol %9s : Checksum 0x%04x\n",
                szProto[v4hdr->ip_protocol], v4hdr->ip_checksum);
    else
        printf("IPv4: Protocol %9d : Checksum 0x%04x\n",
                v4hdr->ip_protocol, v4hdr->ip_checksum);

    sa4.sin_family = AF_INET;
    sa4.sin_port   = 0;
    sa4.sin_addr.s_addr = v4hdr->ip_srcaddr;
    FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), srcaddr, INET_ADDRSTRLEN);

    sa4.sin_addr.s_addr = v4hdr->ip_destaddr;
    FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), destaddr, INET_ADDRSTRLEN);

    printf("IPv4: Source Address       %s\nIPv4: Destination Address  %s\n",
            srcaddr, destaddr);

}

//
// Function: PrintUdpHeader
//
// Description:
//    Prints the UDP header in readable form to the console.
//
void
PrintUdpHeader(
    UDP_HDR *udphdr,
    int len
    )
{
    UNREFERENCED_PARAMETER( len );

    printf("UDP: Source Port  %6d : Destination Port %6d\n",
            ntohs(udphdr->src_portno), ntohs(udphdr->dest_portno));
    printf("UDP: Length     %8d : Checksum        0x4%x\n",
            ntohs(udphdr->udp_length), udphdr->udp_checksum);
}

//
// Function: PrintTcpHeader
//
// Description:
//    Prints the TCP header in readable form to the console.
//
void
PrintTcpHeader(
    TCP_HDR *tcphdr,
    int len
    )
{
    USHORT flags = 0;

    UNREFERENCED_PARAMETER( len );

    printf("TCP: Source Port      %6d : Destination Port %6d\n",
            ntohs(tcphdr->src_portno), ntohs(tcphdr->dest_portno));
    printf("TCP: Sequence Num %10lu : ACK %10lu\n",
            ntohl(tcphdr->seq_num), ntohl(tcphdr->ack_num));
    printf("TCP: Length           %6d : Flags ",
            ((ntohs(tcphdr->lenflags) >> 12) * 4));

    flags = (ntohs(tcphdr->lenflags) & 0x3F);
    if (flags & 0x20)
        printf("URG ");
    if (flags & 0x10)
        printf("ACK ");
    if (flags & 0x08)
        printf("PSH ");
    if (flags & 0x04)
        printf("RST ");
    if (flags & 0x02)
        printf("SYN ");
    if (flags & 0x01)
        printf("FIN ");
    printf("\n");
    printf("TCP: Window Size  %10d : Checksum 0x%04x : Urgent Pointer 0x%04x\n",
            ntohs(tcphdr->window_size), ntohs(tcphdr->tcp_checksum),
            ntohs(tcphdr->tcp_urgentptr));
}

//
// Function: PrintIgmpHeader
//
// Description:
//    This routine prints an IGMP header to the console. For IGMPv1 and IGMPv2
//    this is fairly trivial as the headers are mostly the same and are the
//    same size. For IGMPv3 this is considerably more difficult as the packets
//    can be variable length which requires many additional checks to make sure
//    the packet lengths are consistent.
//
void
PrintIgmpHeader(
    IGMP_HDR *igmphdr,
    int len
    )
{
    SOCKADDR_IN sa4 = {0};
    char        addrbuf[INET_ADDRSTRLEN] = {'\0'};
    int         i = 0;

    if (
#ifdef INEFFICIENT
        // In case you need to modify the code, the following conditional will
        //    distinguish between individual messages and versions...

        // IGMP membership query v1
        ( (igmphdr->version_type == 0x11) && (igmphdr->max_resp_time == 0) && (len == sizeof(IGMP_HDR)) ) ||

        // IGMP membership report v1
        ( (igmphdr->version_type == 0x12) && (igmphdr->max_resp_time == 0) && (len == sizeof(IGMP_HDR)) ) ||

        // IGMP membership query v2
        ( (igmphdr->version_type == 0x11) && (igmphdr->max_resp_time != 0) && (len == sizeof(IGMP_HDR)) ) ||

        // IGMP membership report v2
        ( (igmphdr->version_type == 0x16) && (igmphdr->max_resp_time != 0) && (len == sizeof(IGMP_HDR)) ) ||

        // IGMP leave group (v2)
        ( igmphdr->version_type == 0x17 )
#else
        ( len == sizeof(IGMP_HDR) )
#endif
       )
    {
        if ( (LO_BYTE(igmphdr->version_type) >= 0) && 
             (LO_BYTE(igmphdr->version_type)  < (sizeof(szIgmpType) / sizeof(char *)))
             )
            printf("IGMP: Version         %2d : Type %16s : Maximum Response Time %d\n",
                    HI_BYTE(igmphdr->version_type), szIgmpType[LO_BYTE(igmphdr->version_type)],
                    igmphdr->max_resp_time);
        else
            printf("IGMP: Version         %2d : Type %16d : Maximum Response Time %d\n",
                    HI_BYTE(igmphdr->version_type), LO_BYTE(igmphdr->version_type),
                    igmphdr->max_resp_time);

        // Print the group address
        sa4.sin_family = AF_INET;
        sa4.sin_addr.s_addr = igmphdr->group_addr;
        FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), addrbuf, INET_ADDRSTRLEN);
        printf("IGMP: Checksum    0x%04x : Group      %s\n", 
                igmphdr->checksum, addrbuf);
    }
    else if ( (igmphdr->version_type == 0x11) && (len > sizeof(IGMP_HDR)) )
    {
        IGMP_HDR_QUERY_V3 *query=NULL;

        //
        // IGMP membership query v3
        //
       
        query = (IGMP_HDR_QUERY_V3 *)igmphdr;

        // Verify data is big enough to contain the entire packet
        if ( (len < sizeof(IGMP_HDR_QUERY_V3)) ||
             (len < (int) (sizeof(IGMP_HDR_QUERY_V3) + (sizeof(ULONG) * (query->num_sources-1))) )
           )
        {
            printf("IGMP: Invalid packet\n");
            return;
        }

        printf("IGMP: Version           3 : Type Membership Query : Maximum Response Time %d\n",
                query->max_resp_time);

        // Print the group address
        sa4.sin_family = AF_INET;
        sa4.sin_addr.s_addr = query->group_addr;
        FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), addrbuf, INET_ADDRSTRLEN);
        printf("IGMP: Checksum    0x%04x : Group      %s\n", 
                query->checksum, addrbuf);

        printf("IGMP: Resv 0x%1x : S 0x%1x : QRV 0x%1x : QQIC 0x%02x\n",
                (query->resv_suppr_robust >> 4),
                ((query->resv_suppr_robust >> 3) & 0x1),
                (query->resv_suppr_robust && 0x7),
                query->qqi
                );
        printf("IGMP: Source Count: %lu\n",
                ntohl(query->num_sources));

        // Print out the array of source addresses
        for(i=0; i < (int)ntohl(query->num_sources) ;i++)
        {
            sa4.sin_addr.s_addr = query->sources[i];
            FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), addrbuf, INET_ADDRSTRLEN);
            printf("IGMP: Source [%02d] %s\n", i, addrbuf);
        }

    }
    else if ( (igmphdr->version_type == 0x22) && (len > sizeof(IGMP_HDR)) )
    {
        IGMP_HDR_REPORT_V3   *report=NULL;
        IGMP_GROUP_RECORD_V3 *group=NULL;
        ULONG                 totalrecordslen = 0,
                              recordlen = 0,
                              offset = 0;
        char                 *ptr=NULL;
        int                   j = 0;

        //
        // IGMP membership report v3
        //

        report = (IGMP_HDR_REPORT_V3 *)igmphdr;

        // Safe to touch these fields as we already know the packet is at least
        //    8 bytes in length.
        printf("IGMP: Membership Report : Reserved          0x%02x : Checksum 0x%04x\n",
                report->reserved1, report->checksum);
        printf("IGMP: Resereved  0x%04x : Number of Records %d\n",
                report->reserved2, ntohs(report->num_records));

        // Get a pointer to start of records;
        ptr = (char *)report + sizeof(IGMP_HDR_REPORT_V3);
        offset = 0;
        i = 0;

        totalrecordslen = len - sizeof(IGMP_HDR_REPORT_V3);

        // Go through each of the IGMP group records
        while (offset < totalrecordslen)
        {
            // Get a pointer to the group record
            group = (IGMP_GROUP_RECORD_V3 *) &ptr[offset];

            // Make sure we have enough for group record header (note: we subtract
            //    off the size of the one source which may or may not be present).
            //    The goal is to make sure we can safely read the num_sources field.
            if ((offset + sizeof(IGMP_GROUP_RECORD_V3) - sizeof(ULONG)) > totalrecordslen)
            {
                printf("IGMP: Invalid packet\n");
                break;
            }

            // Compute size of this record
            recordlen =  sizeof(IGMP_GROUP_RECORD_V3) + 
                        (sizeof(ULONG) * (ntohs(group->num_sources) - 1)) + 
                         group->aux_data_len;
            offset += recordlen;

            // Make sure we have enough for the whole record
            if (offset > totalrecordslen)
            {
                printf("IGMP: Invalid packet\n");
                break;
            }

            // Make sure 'type' is in the array of strings if not just print the
            //    integer value
            if ((group->type >= 0) && (group->type < (sizeof(szIgmpMode) / sizeof(char *))) )
                printf("IGMP RECORD [%02d]: Type %16s : Aux Data Len  %d\n",
                        i, szIgmpMode[group->type], group->aux_data_len);
            else
                printf("IGMP RECORD [%02d]: Type %16d : Aux Data Len  %d\n",
                        i, group->type, group->aux_data_len);

            // Print the group address
            sa4.sin_family = AF_INET;
            sa4.sin_addr.s_addr = group->group_addr;
            FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), addrbuf, INET_ADDRSTRLEN);
            printf("IGMP RECORD [%02d]: Number of Sources %9d : Group Address %s\n",
                    i, ntohs(group->num_sources), addrbuf);

            // Print the list of sources
            for(j=0; j < ntohs(group->num_sources) ;j++)
            {
                sa4.sin_addr.s_addr = group->source_addr[j];
                FormatAddress((SOCKADDR *)&sa4, sizeof(sa4), addrbuf, INET_ADDRSTRLEN);
                printf("IGMP_RECORD [%02d]: Source [%02d] %s\n",
                        i, j, addrbuf);
            }

            i++;
        }
    }
    else
    {
        printf("IGMP: Invalid packet\n");
    }
}

//
// Function: CheckForMatchIPv4
//
// Description:
//    This routine determines whether the received packet matches the filter
//    specified. The source and destination IPv4 addresses as well as the 
//    source and destination ports can be filtered on. Note that for protocols
//    that don't contain port numbers (e.g. IGMP) then the port comparison
//    portion of the code will automatically fail.
//
int
CheckForMatchIPv4(
    unsigned int filtermask, 
    SOCKADDR *srcfilter,
    SOCKADDR *destfilter,
    IPV4_HDR *hdr, 
    char *nexthdr
    )
{
    PORT_HDR *ports=NULL;
    int      match = 0;

    match = 1;
    ports = (PORT_HDR *)nexthdr;

    // Try to match source address if specified
    if ((match) && (filtermask & FILTER_MASK_SOURCE_ADDRESS))
    {
        match = !memcmp(
               &((SOCKADDR_IN *)srcfilter)->sin_addr.s_addr,
               &hdr->ip_srcaddr,
                sizeof(unsigned long)
                );
    }
    // Try to match the destination address if specified
    if ((match) && (filtermask & FILTER_MASK_DESTINATION_ADDRESS))
    {
        match = !memcmp(
               &((SOCKADDR_IN *)destfilter)->sin_addr.s_addr,
               &hdr->ip_destaddr,
                sizeof(unsigned long)
                );
    }
    // Try to match source port if specified and ports is not NULL (i.e. its TCP or UDP)
    if ((match) && (ports) && (filtermask & FILTER_MASK_SOURCE_PORT))
    {
        match = ( ((SOCKADDR_IN *)srcfilter)->sin_port == ports->src_portno );
    }
    else if ((ports == NULL) && (filtermask & FILTER_MASK_SOURCE_PORT))
    {
        match = 0;
    }

    // Try to match destination port if specified and ports is not NULL (i.e. its TCP or UDP)
    if ((match) && (ports) && (filtermask & FILTER_MASK_DESTINATION_PORT))
    {
        match = ( ((SOCKADDR_IN *)destfilter)->sin_port == ports->dest_portno );
    }
    else if ((ports == NULL) && (filtermask & FILTER_MASK_DESTINATION_PORT))
    {
        match = 0;
    }

    return match;
}

//
// Function: DecodeIPHeader
//
// Description:
//    This function takes a pointer to an IP header and prints
//    it out in a readable form.
//
int 
DecodeIPHeader(
    char         *buf, 
    int           buflen,
    unsigned long filtermask,
    SOCKADDR     *srcfilter,
    SOCKADDR     *destfilter
    )
{
    IPV4_HDR    *ipv4hdr=NULL;
    UDP_HDR     *udphdr=NULL;
    TCP_HDR     *tcphdr=NULL;
    IGMP_HDR    *igmphdr=NULL;
    BOOL         bFilterMatch = FALSE;
    int          ip_version = 0,
                 ipv4_header_len = 0;

    printf("buflen = %d\n", buflen);

    // Make sure we can at least extract the IP version
    if (buflen < sizeof(char))
    {
        return SOCKET_ERROR;
    }

    // Check the IP version
    ip_version = HI_BYTE(*buf);

    if (ip_version == 4)
    {
        // Verify the buffer is large enough
        if (buflen < sizeof(IPV4_HDR))
        {
            fprintf(stderr, "Received buffer too small!\n");
            return SOCKET_ERROR;
        }

        // Get length of IPv4 header to determine where next protocol header begins
        ipv4_header_len = LO_BYTE(*buf) * 4;

        ipv4hdr = (IPV4_HDR *)buf;

        bFilterMatch = TRUE;

        // Test for match (if filter specified) and get pointer to next protocol header
        switch (ipv4hdr->ip_protocol)
        {
            case IPPROTO_UDP:

                udphdr = (UDP_HDR *) &buf[ipv4_header_len];
                if (filtermask)
                    bFilterMatch = CheckForMatchIPv4(filtermask, srcfilter, destfilter, ipv4hdr, (char *)udphdr);
                break;

            case IPPROTO_TCP:

                tcphdr = (TCP_HDR *) &buf[ipv4_header_len];
                if (filtermask)
                    bFilterMatch = CheckForMatchIPv4(filtermask, srcfilter, destfilter, ipv4hdr, (char *)tcphdr);
                break;

            case IPPROTO_IGMP:

                igmphdr = (IGMP_HDR *) &buf[ipv4_header_len];
                if (filtermask)
                    bFilterMatch = CheckForMatchIPv4(filtermask, srcfilter, destfilter, ipv4hdr, NULL);
                break;

            default:

                // Don't know about this protocol so if we're filtering, don't print it
                if (filtermask)
                    bFilterMatch = FALSE;

                break;
        }

        // If this packet matches, then print it
        if (bFilterMatch)
        {
            PrintIpv4Header(ipv4hdr, buflen);

            switch (ipv4hdr->ip_protocol)
            {
                case IPPROTO_UDP:

                    PrintUdpHeader(udphdr, buflen - ipv4_header_len);
                    break;

                case IPPROTO_TCP:

                    PrintTcpHeader(tcphdr, buflen - ipv4_header_len);
                    break;

                case IPPROTO_IGMP:

                    PrintIgmpHeader(igmphdr, buflen - ipv4_header_len);
                    break;

                default:

                    printf("   No decoder for protocol: %d\n", ipv4hdr->ip_protocol);
                    break; 

            }
            printf("\n");
        }

    }
    else if (ip_version == 6)
    {
        fprintf(stderr, "Currently don't support decoding IPv6 headers\n");
        return SOCKET_ERROR;
    }

    return NO_ERROR;
}
