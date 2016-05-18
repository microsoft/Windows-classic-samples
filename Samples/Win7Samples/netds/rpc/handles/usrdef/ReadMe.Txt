USRDEF


The USRDEF program demonstrates the use of user-defined handles.

The directory samples\rpc\handles\usrdef contains the following files
for building the sample distributed application USRDEF:

File          Description

README.TXT    Readme file for the USRDEF sample
USRDEF.IDL    Interface definition language file
USRDEF.ACF    Attribute configuration file
USRDEFC.C     Client main program
USRDEFS.C     Server main program
USRDEFP.C     Remote procedures
MAKEFILE      Nmake file to build for Windows 

-------------------------------------------
BUILDING CLIENT AND SERVER APPLICATIONS 
-------------------------------------------

The following environment variables should be set for you already.

  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin;

Where %SDKROOT% is the root directory for the 32-bit Windows SDK.

For mips, set CPU=mips
For alpha, set CPU=alpha

Build the sample distributed application:
  nmake cleanall
  nmake

This builds the executable programs usrdefc.exe
(client) and usrdefs.exe (server).

------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter

  usrdefs

On the client, enter

  net start workstation
  usrdefc

Note:  The client and server applications can run on
the same Microsoft Windows NT computer when you use
different screen groups.

Several command line switches are available to change
settings for this program. For a listing of the switches
available from the client program, enter

  usrdefc -?

For a listing of switches available from the server
program, enter

  usrdefs -?
