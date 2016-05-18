RPCSVC


The RPCSVC program demonstrates how to implement an NT system service that 
uses RPC.

SUMMARY
=======

This particular RPC service is designed to show the performance effects of 
various parameters to RPC APIs and interface design.

The service can be started and stopped from either the Services control 
panel, the net start/stop command line, or by using the Service Controller 
utility (see below).

The service also provides command-line parameters which install, remove, or 
run (debug) the service as a console application.

This sample (and service.c) is based on the win32\service sample.

FILES
=====

The directory samples\rpc\handles\rpcsvc contains the following files to
build the distributed application RPCSVC:

File          Description

README.TXT    Readme file for the RPCSVC sample
RPCSVC.IDL    Interface definition language file
RPCSVC.ACF    Attribute configuration file
CLIENT.C      Client main program
SERVER.C      Server main program
SERVICE.C     Service APIs
SERVICE.H
MAKEFILE      Nmake file to build for Windows NT or Windows 95

TO USE:
-------

To install the service, first compile everything, and then type:

  rpcsvc -install

To start the service, use the control panel or type:

  net start simplerpcservice

Once the service has been started, you can use the CLIENT program to verify 
that it really is working, using the syntax:

  svcclnt [-i iterations] [-t protseq] [-n servername] [-s security]

By default, the service listens to ncalrpc, ncacn_np, ncacn_ip_tcp and 
ncadg_ip_udp.  If no parameters are passed to the client, it uses ncalrpc 
with default LPC security.
    
If, after playing with the sample you wish to remove the service, simply 
run:

  rpcsvc -remove

You can run the service from the command line with:

  rpcsvc -debug

MORE INFORMATION:
=================

See the sample in samples\win32\service for more information on NT system 
service and how service.c and service.h work.

server.c implements a bunch of APIs:

    ServiceStart() is called when the server is starting. It uses server 
    transports, registers its endpoints and interfaces and starts listening.

    ServiceStop() is called when the service should stop. It just calls 
    RpcMgmtStopServerListening().

    Manager APIs (implement the remote side of the operations defined in the 
    rpcsvc.idl file.

Client.c binds to the server (as specified on the command line) and then 
times a bunch of different RPC calls to the server.
