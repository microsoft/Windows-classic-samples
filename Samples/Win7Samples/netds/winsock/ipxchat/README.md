---
page_type: sample
languages:
- c
products:
- windows-api-win32
name: IPXCHAT sample
urlFragment: ipxchat-example-application
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: This program demonstrates the use of Windows Sockets to communicate over the IPX/SPX protocol.
---

# IPXCHAT example application

This program demonstrates the use of Windows Sockets to communicate over the IPX/SPX protocol.

Setup:

IPXCHAT requires that an IPX protocol is loaded.  The "IPX/SPX Compatible Transport" shipped in the box is the protocol which was used for testing this application.  Make sure that the protocol is configured correctly for the same frame type on the two machines which are talking to each other.

To use IPXCHAT:

There are two sides to a sockets application, the server side (the listening side), and the client side (the calling side).  On the first
machine you will need to set up IPXCHAT as the server.  

Start IPXCHAT and select Options->Listen.

You will be prompted to enter a socket number.  A default of 2FFF will be offered to you although you may change it to anything you want.  Be aware that there are well known XNS sockets which you should stay away from.  See Novell's "IPX Router Specification" for a list of these.

When you select OK, IPXCHAT will initialize Windows Sockets and start listening on the specified socket.  In the status box of the dialog you
will see a message "listening on <netnum>.<nodenum>.<socknum>" where netnum will be a number specifying the network number, nodenum specifies
the node number, and socknum specifies the socket number.  You will need these numbers to enter on the client side in order to connect.

On the second machine (actually it can be the same machine as the server):

Start IPXCHAT and select Options->Connect.

You will be prompted to enter the Network Number, Node Number, and Socket Number indicated on the server (the three numbers in the status box of the server's listen dialog).  Once you connect you should be able to type in the top edit control and it should appear on the bottom of your counterpart's edit control.
