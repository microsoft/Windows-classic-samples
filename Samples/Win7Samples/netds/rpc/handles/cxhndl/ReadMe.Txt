CXHNDL


The CXHNDL program demonstrates the use of the context_handle attribute.

The directory samples\rpc\handles\cxhndl contains the following files for
building the sample distributed application CXHNDL:

File          Description

README.TXT    Readme file for the CXHNDL sample
CXHNDL.IDL    Interface definition language file
CXHNDL.ACF    Attribute configuration file
CXHNDLC.C     Client main program
CXHNDLS.C     Server main program
CXHNDLP.C     Remote procedures
MAKEFILE      Nmake file to build for Windows NT 

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

This builds the executable programs cxhndlc.exe
(client) and cxhndls.exe (server).

------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  cxhndls

On the client, enter:

  net start workstation
  cxhndlc

Note:  The client and server applications can run on the same Microsoft 
Windows NT computer when you use different screen groups.

Several command-line switches are available to change settings for this 
program. For a listing of the switches available from the client program, 
enter:

  cxhndlc -?

For a listing of switches available from the server program, enter:

  cxhndls -?
