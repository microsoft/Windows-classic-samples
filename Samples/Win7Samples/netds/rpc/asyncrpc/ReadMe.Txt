AsyncRPC Sample

======
SUMMARY
======
The AsyncRPC program demonstrates the following asynchronous
RPC features:

  [async] attribute
  RPC_ASYNC_STATE
  RPC_ASYNC_EVENT
  RpcAsyncInitializeHandle()
  RpcAsyncCancelCall
  RpcServerTestCancel
  RpcAsyncGetCallHandle
  RpcAsyncCompleteCall()
  Notification using event, I/O completion port,
    or asychronous precedure call


=====
FILES
=====

The directory contains the following files for building
the sample distributed application AsyncRPC:

File          Description

README.TXT    Readme file for the AsyncRPC sample
AsyncRPC.IDL  Interface definition language file
AsyncRPC.ACF  Attribute configuration file
AsyncRPCC.C   Client main program
AsyncRPCS.C   Server main program
AsyncRPCP.C   Remote procedures
MAKEFILE      Nmake file to build for Windows NT or Windows 95

=================
PLATFORM SUPORTED
=================

Windows NT 5 or later.

To build, type "nmake" at the command line.

The following environment variables should be already set:
  
  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin

where %SDKROOT% is the root directory for the 32-bit Windows SDK.

For mips, set CPU=mips
For alpha, set CPU=alpha

Build the sample distributed application:

  nmake cleanall
  nmake

This builds the executable programs AsyncRPCc.exe (client) and 
AsyncRPCs.exe (server).

=============================
RUNNING THE CLIENT AND SERVER
=============================
On the server, enter:

  AsyncRPCs

On the client, enter:

  AsyncRPCc -n ServerName

Note: The client and server applications can run on the same 
Microsoft Windows NT computer when you use different screen groups.

Several command-line switches are available to change settings for 
this program. For a listing of the switches available from the client 
program, enter:

  AsyncRPCc -?

For a listing of switches available from the server
program, enter:

  AsyncRPCs -?
