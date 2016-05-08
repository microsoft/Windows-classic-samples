LoadProtocol

LoadProtocol is a console application that allows you to load the sample IP Routing Protocol that ships with the SDK. (Please refer to ..\IP\ for the Routing Protocol sample).

FILES
=====

The LoadProtocol application uses the following files

File            Description

README.TXT      Readme file for LoadProtocol application
LoadProtocol.C  C source code
ipsamplerm.h	Header file that contains Sample IP Routing Protocol specifics.
MAKEFILE        Nmake file for Windows 2000



-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS 2000:
-------------------------------------------

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program LoadProtocol.exe
for Microsoft Windows 2000.


--------------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS 2000
--------------------------------------------------------

To run the application type:

    LoadProtocol

-------------------------
ADDITIONAL INFORMATION
-------------------------

You will need to create the Ipsample.dll using the ..\IP\ folder sample. Copy the Ipsample.dll into the %SystemRoot%\system32 folder. Also register the IP Sample Protocol using the Ipsample.reg file.

To check whether the Sample Protocol is loaded, type in the command prompt:
netsh routing ip show protocol

The output will show:
Unicast		MS-0000		200

This confirms that the protocol was loaded.


