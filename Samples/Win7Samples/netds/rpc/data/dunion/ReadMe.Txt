DUNION


The DUNION program illustrates discriminated unions.

FILES
=====

The directory samples\rpc\handles\data\dunion contains the following files
to build the distributed application DUNION:

File          Description

README.TXT    Readme file for the DUNION sample
DUNION.IDL    Interface definition language file
DUNION.ACF    Attribute configuration file
DUNIONC.C     Client main program
DUNIONS.C     Server main program
DUNIONP.C     Remote procedures
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

This builds the executable programs dunionc.exe
(client) and dunions.exe (server).


------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  dunions

On the client, enter:

  net start workstation
  dunionc

Note:  The client and server applications can run on the same Microsoft 
Windows NT computer when you use different screen groups.

Several command line switches are available to change settings for this 
program. For a listing of the switches available from the client program, 
enter:

  dunionc -?

For a listing of switches available from the server program, enter:

  dunions -?
