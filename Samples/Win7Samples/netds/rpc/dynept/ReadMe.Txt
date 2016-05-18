DYNEPT


The DYNEPT program demonstrates a client application managing its connection 
to the server through dynamic endpoints.

FILES
=====

The directory samples\rpc\dynept contains the following files for building
the distributed application DYNEPT:

File          Description

README.TXT     Readme file for the DYNEPT sample
DYNEPT.IDL     Interface definition language file
DYNEPT.ACF     Attribute configuration file
DYNEPTC.C      Client main program
DYNEPTS.C      Server main program
DYNEPTP.C      Remote procedures
MAKEFILE       Nmake file to build for Windows 

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

This builds the executable programs dyneptc.exe
(client) and dynepts.exe (server).


------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  dynepts

On the client, enter:

  net start workstation
  dyneptc

Note:  The client and server applications can run on the same Microsoft 
Windows NT computer when you use different screen groups.

Several command-line switches are available to change settings for this 
program. For a listing of the switches available from the client program, 
enter:

  dyneptc -?

For a listing of switches available from the server
program, enter:

  dynepts -?
