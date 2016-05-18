readme.txt

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998  Microsoft Corporation.  All Rights Reserved.


About:

This directory contains 3 programs, an I/O Completion Port (IOCP) Winsock 
server using WSAAccept (iocpserver), an IOCP Winsock server using AcceptEx
(iocpserverex) and a simple multithreaded Winsock client (iocpclient)
used to test either server.  Unless otherwise noted, comments about iocpserver
apply to iocpserverex as well.

Iocpserver is a simple echo program that uses IOCP to receive data from and 
echo data back to a sending client. The server program supports multiple 
clients connecting via TCP/IP and sending arbitrary sized data buffers which 
the server then echoes back to the client.  For convenience a very simple 
client program, iocpclient, was developed to connect and continually send 
data to the server to stress it using multiple threads (NOTE: please see 
the comments in iocpclient.cpp about scaleability of multithreaded 
applications).

Direct IOCP support was added to Winsock 2 and is fully implemented on the 
NT platform.  IOCP support is not available on Win9x platforms.  IOCPs in 
Winsock2 are modeled on the Win32 IOCP model and compatible with them. IOCPs 
provide a model for developing very high performance and very scablable 
server programs.  As a consequence, iocpserver can only run on NT.  Use
of IOCP-capable functions in Winsock 2 (such as WSASend and WSARecv) is
preferred over using IOCPs in WriteFile and ReadFile when using sockets.  
Additionally, the client is designed to run only on NT as well. 

The basic idea is that this server continuously accepts connection requests 
from a client program.  When this happens, the accepted socket descriptor 
is added to the existing IOCP and an inital receive (WSARecv) is posted on 
that socket.  When the client then sends data on that socket, an I/O packet 
will be delivered and handled by one of the server's worker threads.  The 
worker thread echoes the data back to the sender by posting a send (WSASend) 
containing all the data just received.  When sending the data back to the 
client completes, another I/O packet will be delivered and again handled by 
one of the server's worker threads.  Assuming all the data that needed to be 
sent was actually sent, another receive (WSARecv) is once again posted and 
the scenario repeats itself until the client stops sending data.
		
The client and server can be shut down by pressing CTRL-C. The server can
restart without exiting the process by pressing CTL-BRK.

The basic difference between iocpserver and iocpserverex is that AcceptEx (used
in iocpserverex) allows data to be returned from an accepted connection and AcceptEx 
can be executed in an overlapped manner and thus be used in conjunction with 
an IOCP.



Build:

    Run nmake to use the supplied makefile or create a VC project for 
    iocpserver, iocpserverex, and iocpclient.   When creating a project in VC 
    remember to link with the Winsock 2 library ws2_32.lib.  Also, use the 
    headers and libs from the April98 Platform SDK or later.


Usage:

    Start the server and wait for connections on port 6001
	iocpserver -e:6001

    Start the client with 32 threads in verbose mode and connect to port 6001
    on the server machine, server_machine.
        iocpclient -n:server_machine -t:32 -v -e:6001

