SUMMARY
======= 
 
The IPHDRINC samples demonstrate the use of raw sockets and the header include
option for both IPv4 and IPv6. It does this by sending raw UDP packets and by
setting the IP header include option. This means each raw send has to include
the IP and UDP header before the data payload. This sample allows you to set
the source IP and port in the outgoing IP header to an arbitrary value.

Also note that the "raw" behavior between the IPv4 and IPv6 stacks are not
entirely equal. The IPv4 stack will fragment the data as necessary (i.e.
you have no control over fragmentation) while the IPv6 stack assumes the
caller will do all the fragmentation required (attempting to send something
larger than the MTU will fail).


DISCLAIMER
==========
From Network Programming for Microsoft Windows, Second Edition by Anthony
Jones and James Ohlund.  Copyright 2002.  Reproduced by permission of
Microsoft Press.  All rights reserved. 


FILES
=====

README.TXT	Readme file
MAKEFILE	Makefile file
RAWUDP.C	Source for sending raw UDP packets
RESOLVE.C	Common routines for resolving and printing addresses


PLATFORM SUPORTED
=================

Windows 2000 and later. The IP_HDRINCL option is availalbe only on Windows
2000 and later. The IPV6_HDRINCL option is supported on Windows XP and later
(the IPv6 stack must be installed).


RUNNING THE SERVER AND CLIENT APPLICATIONS
==========================================

To build, type "nmake" at the command line.

To run the samples, please follow the usage specified below

usage: rawudp.exe [options]

    Options:
        -a  4|6    Address family
        -sa addr   From (sender) port number
        -sp int    From (sender) IP address
        -da addr   To (recipient) port number
        -dp int    To (recipient) IP address
        -n  int    Number of times to read message
        -m  str    String message to fill packet data with
        -p  proto  Protocol value
        -r  port   Receive raw (SOCK_RAW) datagrams on the given port
        -rd port   Receive datagram (SOCK_DGRAM) on the given port
        -t  mtu    MTU size (required for fragmentation)
        -z  int    Size of message to send

For IPv4:
    To receive packets on interface x.y.z.w and port 5000:
        iphdrinc.exe -sa x.y.z.w -rd 5000

    To send raw UDP packets to be received by the receiver above:
        iphdrinc.exe -sa 1.2.3.4 -sp 1234 -da x.y.z.w -dp 5000

For IPv6:
    To receive packets on interface fe80::xyzw%a and port 5000:
        iphdrinc.exe -sa fe80::xyzw%a -rd 5000 

    To send raw UDP packets to be received by the receiver above:
        iphdrinc.exe -sa fe80::1234 -sp 1234 -da fe80::xyzw%b -dp 5000

        NOTE: The sender's destination scope-id (%b) must be the interface
              index that the receive is located on.
