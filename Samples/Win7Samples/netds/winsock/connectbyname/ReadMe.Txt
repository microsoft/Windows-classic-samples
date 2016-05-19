connectbyname.cpp
-----------------

This IPv6 sample demonstrates the use of WSAConnectByName.

WSAConnectByName is new to Windows Sockets in Windows Vista.

This sample requires that TCP/IP version 6 be installed on the system (default
configuration for Windows Vista).

IPv6 socket calls WSAConnectByName to resolve name and connect to listener in 
a single API call. The local and remote addresses are also returned. The connecting
entity sends some data after WSAConnectByName. The accepting entity receives the
peer data. Then both sides close the connection. 

Usage:
ConnectByName.exe
