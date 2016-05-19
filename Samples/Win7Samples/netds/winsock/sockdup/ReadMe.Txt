SUMMARY
======= 
 
The SOCKDUP sample demonstrates the basics of shared socket programming in Winsock 2 
using WSADuplicateSocket and WSASocket. The parent and child processes use memory mapped 
file to exchange the socket protocol information (WSAPROTOCOL_INFO) needed to duplicate 
the socket in the child process.

The server application can function as either the parent or a child.  The parent accepts 
each client connection request and dispatches a child process to handle the actual I/O 
for the connection.  After a child process is successfully created, the parent is free to 
handle other client connection requests.


FILES
=====

README.TXT    		Readme file
MAKEFILE      		Makefile file
client\SOCKDUPC.C	Client main source
server\SOCKDUPS.C    	Server main source


PLATFORM SUPORTED
=================

Windows NT 4 or later


RUNNING THE SERVER AND CLIENT APPLICATIONS
==========================================

To build, type "nmake" at the command line.

The client and server applications can run on the same Microsoft Windows NT or 9x computer 
when you use different screen groups.

To run the server, type:

  SOCKDUPS

To run the client, type:

  SOCKDUPC -l N

or

  SOCKDUPC -n BIGTOWER -l N

where BIGTOWER is the computer name and N specifies the number of messages you wish to 
send to the server.

For more information about the command line switches available from the server, enter:

  SOCKDUPS -?

For a listing of switches available from the client, enter:

  SOCKDUPC -?
