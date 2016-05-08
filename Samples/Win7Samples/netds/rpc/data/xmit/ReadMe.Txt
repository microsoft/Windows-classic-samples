XMIT


The XMIT program demonstrates the type-conversion attribute transmit_as.

FILES
=====

The directory samples\rpc\data\xmit contains the following files for
building the sample distributed application XMIT:

File          Description

README.TXT    Readme file for the XMIT sample
XMIT.IDL      Interface definition language file
XMIT.ACF      Attribute configuration file
XMITC.C       Client main program
XMITS.C       Server main program
XMITP.C       Remote procedures
XMITU.C       Utility functions for client, server
XMITU.H       Utility function prototypes
MAKEFILE      Nmake file to build for Windows NT or Windows 95
MAKEFILE.DOS  Nmake file to build for MS-DOS

-------------------------------------------
BUILDING CLIENT AND SERVER APPLICATIONS FOR
MICROSOFT WINDOWS NT OR WINDOWS 95:
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

This builds the executable programs xmitc.exe
(client) and xmits.exe (server).

-----------------------------------------------------------------------
BUILDING THE CLIENT APPLICATION FOR MS-DOS
-----------------------------------------------------------------------

After installing the Microsoft Visual C/C++ version 1.50 development
environment and the 16-bit RPC SDK on a Windows NT or Windows 95
computer, you can build the sample client application from Windows NT
or Windows 95.

  nmake -f makefile.dos cleanall
  nmake -f makefile.dos

This builds the client application xmitc.exe.

You may also execute the Microsoft Visual C/C++ compiler under MS-DOS.
This requires a two step build process.

  Step One: Compile the .IDL files under Windows NT or Windows 95
     nmake -a -f makefile.dos xmit.idl

  Step Two: Compile the C sources (stub and application) under MS-DOS
     nmake -f makefile.dos

------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter

  xmits

On the client, enter

  net start workstation
  xmitc

Note:  The client and server applications can run on
the same Microsoft Windows NT computer when you use
different screen groups.

Several command line switches are available to change
settings for this program. For a listing of the switches
available from the client program, enter

  xmitc -?

For a listing of switches available from the server
program, enter

  xmits -?
