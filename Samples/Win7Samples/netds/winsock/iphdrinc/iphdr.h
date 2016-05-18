// 
// IP Header Definitions
//
// Description:
//    This file contains protocol header definitions.
//
// NOTE:
//    From Network Programming for Microsoft Windows, Second Edition 
//    by Anthony Jones and James Ohlund.  Copyright 2002.  
//    Reproduced by permission of Microsoft Press.  All rights reserved. 
// 
#include <pshpack1.h>

//
// IPv4 Header (without any IP options)
//
typedef struct ip_hdr
{
    unsigned char  ip_verlen;        // 4-bit IPv4 version
                                     // 4-bit header length (in 32-bit words)
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_totallength;   // Total length
    unsigned short ip_id;            // Unique identifier 
    unsigned short ip_offset;        // Fragment offset field
    unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    unsigned int   ip_srcaddr;       // Source address
    unsigned int   ip_destaddr;      // Source address
} IPV4_HDR, *PIPV4_HDR, FAR * LPIPV4_HDR;

//
// IPv6 Header
//
typedef struct ipv6_hdr
{
    unsigned long   ipv6_vertcflow;        // 4-bit IPv6 version
                                           // 8-bit traffic class
                                           // 20-bit flow label
    unsigned short  ipv6_payloadlen;       // payload length
    unsigned char   ipv6_nexthdr;          // next header protocol value
    unsigned char   ipv6_hoplimit;         // TTL 
    struct in6_addr ipv6_srcaddr;          // Source address
    struct in6_addr ipv6_destaddr;         // Destination address
} IPV6_HDR, *PIPV6_HDR, FAR * LPIPV6_HDR;

//
// IPv6 Fragmentation Header
//
typedef struct ipv6_fragment_hdr
{
    unsigned char   ipv6_frag_nexthdr;      // Next protocol header
    unsigned char   ipv6_frag_reserved;     // Reserved: zero
    unsigned short  ipv6_frag_offset;       // Offset of fragment
    unsigned long   ipv6_frag_id;           // Unique fragment ID
} IPV6_FRAGMENT_HDR, *PIPV6_FRAGMENT_HDR, FAR * LPIPV6_FRAGMENT_HDR;

//
// Define the UDP header 
//
typedef struct udp_hdr
{
    unsigned short src_portno;       // Source port no.
    unsigned short dst_portno;       // Dest. port no.
    unsigned short udp_length;       // Udp packet length
    unsigned short udp_checksum;     // Udp checksum
} UDP_HDR, *PUDP_HDR;

#include <poppack.h>
