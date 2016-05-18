SUMMARY
======= 

The Accept sample demonstrates the basic use of 
	1. Non-blocking accept using select()
	2. Asynchronous accept using WSAAsyncSelect()

The sample includes a server and a client. The server waits for clients to 
connect. For each connected client, the server receives any amount of data that 
the client sends and echoes them all back in the same order. The server can 
simultaneously accept more than one client. The server closes the socket for 
each client as soon as the client is fully done with sending the data and also 
the after the server has completely echoed all the data back unless there's any 
error.

The client simply connects to the server, sends a user-specified amount of data
and receives all the echoed data back from the server and closes the socket 
when all the data has been sent and received or if there's any error.

FILES
=====

README.TXT      Readme file
MAKEFILE        Makefile

Server-side (Accept\Server)
===========
MAIN.CPP        Command-line Argument Processing and main()
NBACCEPT.CPP    Non-blocking Accept implementation 
ASACCEPT.CPP    AsyncSelect Accept implementation
COMMON.CPP	    Events processing common to both accept implementations
SOCKINFO.CPP    Utility functions to manage the Socket context objects
COMMON.H        Common header for all the above files

Client-side (Accept\Client)
===========
TESTACCEPT.CPP  Client to test the server's accept implementation


PLATFORMS SUPORTED
==================

Windows XP or later.


RUNNING THE SERVER AND CLIENT APPLICATIONS
==========================================

To build, type "nmake" at the command line.

The client and server applications can run on the same Microsoft Windows XP 
computer or a different one.

To run the server, type:

  AccServ.exe 
  AccServ.exe -? for other configurable options.

To run the client on the same machine as the server, type:

  TestAcc.exe simplest version.
  TestAcc.exe -? for other configurable options.

To run the server for doing WSAAsyncSelect and to see that it blocks in send 
if the client is not reading yet, the following command-lines can be specified.

  AccServ.exe -t 2
  TestAcc.exe -b 50000 -s 3


