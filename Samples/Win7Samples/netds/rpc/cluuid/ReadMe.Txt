CLUUID


The CLUUID program demonstrates how to supply multiple implementations of
the remote procedure specified in the interface. It also demonstrates
how the client selects among the implementations by providing a client 
object uuid.

SUMMARY
=======

The server calls RpcObjectSetType to associate a client object uuid with 
the object uuid in the Object Registry Table. The server initializes a 
manager entry point vector (manager epv) and then calls RpcRegisterIf to 
associate the interface uuid and the object uuid with the manager epv in 
the Interface Registry Table.

When the client makes a remote procedure call, the client object uuid is 
mapped to the object uuid in the Object Registry Table. The resulting
object uuid and the interface uuid are mapped to a manager entry point 
vector in the Interface Registry Table.

By default, in this example, the server registers two implementations of 
the "hello, world" function HelloProc and HelloProc2. The HelloProc2
implementation is associated with the object uuid
"11111111-1111-1111-1111-111111111111". When the client makes a procedure 
call with a null uuid, the client's request is mapped to the original 
HelloProc. When the client makes a procedure call with the client object 
uuid "11111111-1111-1111-1111-11111111111", the client's request is mapped 
to HelloProc2 (which prints the string in reverse).

FILES
=====

The directory samples\rpc\cluuid contains the following files for
building the sample distributed application CLUUID:

File          Description

README.TXT    Readme file for the cluuid sample
CLUUID.IDL    Interface definition language file
CLUUID.ACF    Attribute configuration file
CLUUIDC.C     Client main program
CLUUIDS.C     Server main program
CLUUIDP.C     Remote procedures
MAKEFILE      Nmake file to build for Windows

-------------------------------------------
BUILDING CLIENT AND SERVER APPLICATIONS 
-------------------------------------------

The following environment variables should be already set for you:

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

This builds the executable programs cluuidc.exe(client) and 
cluuids.exe (server).

------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  cluuids

On the client, enter:

  net start workstation
  cluuidc

To call the second implementation of the function,
on the client, enter:

  cluuidc -u "11111111-1111-1111-1111-111111111111"

Note: The client and server applications can run on the same Microsoft 
Windows NT computer when you use different screen groups.

Several command line switches are available to change settings for this 
program. For a listing of the switches available from the client program, 
enter:

  cluuidc -?

For a listing of switches available from the server program, enter:

  cluuids -?


