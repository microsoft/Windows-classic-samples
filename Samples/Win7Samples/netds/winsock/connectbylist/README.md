---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: WSAConnectByList sample
urlFragment: connect-by-list
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This IPv6 sample demonstrates the use of WSAConnectByList and dual-mode IPv4/IPv6 sockets.
---

# connectbylist.cpp

This IPv6 sample demonstrates the use of WSAConnectByList and dual-mode IPv4/IPv6 sockets.

WSAConnectByList is new to Windows Sockets in Windows Vista.  

This sample requires that TCP/IP version 6 be installed on the system, default configuration for Windows Vista.

The client builds a socket address list and connects to the server. This is done for IPv4 (IPv6 v4-mapped address) and IPv6.

Usage:
ConnectByList.exe
