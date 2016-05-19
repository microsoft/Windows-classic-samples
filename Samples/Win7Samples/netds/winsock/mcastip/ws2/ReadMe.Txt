SUMMARY
======= 
 
The MCASTIP samples demonstrate the basic multicast sequence of operations 
in WinSock 1 (setsocketopt) and WinSock 2 (WSAJoinLeaf). The sample also
illustrates IPv4 and IPv6 multicasting.


DISCLAIMER
==========

 From Network Programming for Microsoft Windows, Second Edition by Anthony Jones 
 and James Ohlund.  Copyright 2002.  
 Reproduced by permission of Microsoft Press.  All rights reserved.


FILES
=====

README.TXT	Readme file
MAKEFILE	Makefile file
MCASTWS1.C	Source for multicast client/server using WinSock 1
MCASTWS2.C	Source for multicast client/server using WinSock 2
RESOLVE.C   Common routines for resolving and printing addresses
RESOLVE.H   Function prototypes for routines in RESOLVE.C

PLATFORM SUPPORTED
=================

Windows NT 4, Windows 2000, and Windows 95/98.  Note that for Windows 95, 
"Winsock 2 Update for WinSock 2" is required in order to run MCASTWS2 
(please see http://support.microsoft.com/support/platformsdk/networking/Downloads.asp).


RUNNING THE SERVER AND CLIENT APPLICATIONS
==========================================

To build, type "nmake" at the command line.

To run the samples, please follow the usage specified below

    mcastws1.exe [options]

    Options:
        -b str String address to bind to
        -c     Connect before sending?
        -i str Local interface to join groups
        -j     Don't join the multicast group
        -l 0/1 Turn on/off loopback
        -m str Multicast address to join
        -n int Number of messages to send/receive
        -p int Port number to use
                 The default port is: 25000
        -r     Use raw sockets
        -s     Act as server (send data); otherwise
                 receive data.
        -t int Set multicast TTL
        -u     Set SO_REUSEADDR
        -z int Size of the send/recv buffer

    mcastws2.exe [options]

    Options:
        -i addr Local address to bind to
        -l 0/1  Disable/enable loopback
        -m addr Multicast address to join
        -n int  Send/recv count
        -p int  Port number
        -s      Act as sender; default is receiver
        -t int  Set multicast TTL
        -z int  Buffer size (in bytes)

