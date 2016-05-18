readme.txt

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All Rights Reserved.


Description:

This sample illustrates how to use the SIO_RCVALL, SIO_RCVALL_MCAST, and
SIO_RECVALL_IGMPMCAST ioctls available on Windows 2000 and later.
These new ioctls allow a Winsock application to capture all IP traffic, 
all IP multicast traffic, or all IGMP traffic on a given network interface. 
This is done by creating a raw socket which is bound to a specific local IP 
interface.  Once the socket is bound, the specified ioctl option is set and 
subseqent receive operations will return incoming IP packets of the requested 
type. Note that these ioctls are currently supported only for IPv4.

Files:

    rcvall.c        - Main program which sets up socket and receives data
    parser.c        - Contains parsing routines for displaying packets
    parser.h        - Function prototypes for parser.c
    iphdr.h         - Protocol header definitions (IPv4, UDP, TCP, etc.)
    resolve.c       - Common routines for parsing/resolving IP addresses
    resolve.h       - Header file for resolve.c

Build:

    Via NMAKE
        Run nmake.exe in the sample directory.

    Via manual build:
        cl rcvall.c parser.c resolve.c ws2_32.lib

Usage:

    This sample works on Windows 2000 and later.

    To run the sample (capture all IPv4 traffic):

        rcvall.exe -i x.y.z.w

    Where x.y.z.w is the IP address of a local interface. To obtain a list
    of local interfaces:

        rcvall.exe --help

    To capture IGMP traffic (SIO_RCVALL_IGMPMCAST) or all multicast traffic
        (SIO_RCVALL_MCAST) use the -t:OPTION argument where OPTION is the
        string "igmp" or "multicast" respectively.

        For example to capture IGMP traffic only:

        rcvall.exe -i x.y.z.w -t igmp

        To capture all multicast traffic only:

        rcvall.exe -i x.y.z.w -t multicast

    You may also filter the received IP traffic based upon source IP address,
        source port, destination IP address, and/or destination port.

        -sa aaa.bbb.ccc.ddd     filters on source IP address
        -da aaa.bbb.ccc.ddd     filters on destination IP address
        -sp x                   filters on source port
        -dp x                   filters on destination port

        For example the following command will listen on the first IP
        interface for all IP traffic whose destination is 10.0.0.1 on port 21:

        rcvall.exe -i x.y.z.w -t ip -da 10.0.0.1 -dp 21

        Only those IP packets received that match the given criteria will
        be printed.
