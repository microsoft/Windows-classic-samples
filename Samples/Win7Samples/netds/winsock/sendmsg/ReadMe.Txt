sendmsg.cpp:
---------
This IPv6 sample demonstrates the use of WSASendMsg() and the IPV6_PKTINFO option
 and WSARecvMsg to retrieve the source address of a datagram.

WSASendMsg is new to Windows Sockets in Windows Vista.
 
This sample requires that TCP/IP version 6 be installed on the system (default
configuration for Windows Vista).

A IPv6 datagram socket is created and bound and the IPV6_PKTINFO option is set. 
Overlapped WSARecvMsg is posted, and data is sent on the socket via sendto. 
Upon completion, the received message is sent again via WSASendMsg.
The data is received again via WSARecvMsg. 

Usage:
SendMsg.exe
