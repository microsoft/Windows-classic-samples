poll.cpp
-----------------

This IPv6 sample demonstrates the use of WSAPoll.

WSAPoll is new to Windows Sockets in Windows Vista.

This sample requires that TCP/IP version 6 be installed on the system (default
configuration for Windows Vista).

The client connects a socket to the server, then sends/recvs when WSAPoll 
indicates such is possible to do without blocking. Likewise for the server (listener).
 
Usage:
WSAPollSample.exe
