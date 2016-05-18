rmmc.c:
---------
This IPv6 sample demonstrates the use of WSARecvMsg() and the IPV6_PKTINFO option
to determine multicast reception on a datagram socket. 

WSARecvMsg is new to Windows Sockets in Windows XP.

This sample requires that TCP/IP version 6 be installed on the system.

A IPv6 datagram socket is created and bound and the IPV6_PKTINFO option is set. 
The socket is joined to multicast group and an overlapped WSARecvMsg is posted. 
MSG_MCAST flag is checked upon completion, upon where further
processing can be made. 

Usage:
Run rmmc.exe
