HELLO


The HELLO program is a client/server implementation of the classic "Hello,
world" program.

FILES
=====

The directory samples\rpc\hello contains the following files for
building the sample distributed application HELLO:

File          Description

README.TXT    Readme file for the HELLO sample
HELLO.IDL     Interface definition language file
HELLO.ACF     Attribute configuration file
HELLOC.C      Client main program
HELLOS.C      Server main program
HELLOP.C      Remote procedures
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

This builds the executable programs helloc.exe (client) and 
hellos.exe (server).

------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  hellos

On the client, enter:

  net start workstation
  helloc

Note: The client and server applications can run on the same 
Microsoft Windows NT computer when you use different screen groups.

Several command-line switches are available to change settings for 
this program. For a listing of the switches available from the client 
program, enter:

  helloc -?

For a listing of switches available from the server
program, enter:

  hellos -?
