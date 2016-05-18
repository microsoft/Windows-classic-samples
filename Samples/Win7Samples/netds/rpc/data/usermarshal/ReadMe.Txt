UserMarshal


The UserMarshal program demonstrates the user_marshal ACF attribute.

FILES
=====

The directory samples\rpc\data\UserMarshal contains the following files for
building the sample distributed application UserMarshal:

File          Description

README.TXT    Readme file for the UserMarshal sample
umarsh.IDL     Interface definition language file
umarshc.ACF    Client version of Attribute configuration file
umarshs.ACF    Server version of Attribute configuration file
umarshc.C      Client main program
umarshs.C      Server main program
umarshp.C      Remote procedures
umarshcu.C     Client Utility functions
umarshsu.C     Server Utility functions
MAKEFILE      Nmake file to build for Windows XP and above

-------------------------------------------
BUILDING CLIENT AND SERVER APPLICATIONS FOR
MICROSOFT WINDOWS XP and above
-------------------------------------------

The following environment variables should already be set for you:

  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin

where %SDKROOT% is the root directory for the 32-bit Windows SDK.


Build the sample distributed application:

  nmake cleanall
  nmake

This builds the executable programs umarshc.exe (client) and 
umarshs.exe (server).


------------------------------------------
RUNNING THE CLIENT AND SERVER APPLICATIONS
------------------------------------------

On the server, enter:

  umarshs

On the client, enter:

  net start workstation
  umarshc

Note:  The client and server applications can run on the same Microsoft 
Windows NT computer when you use different screen groups.

Several command line switches are available to change settings for this 
program. For a listing of the switches available from the client program, 
enter:

  umarshc -?

For a listing of switches available from the server
program, enter:

  umarshs -?
