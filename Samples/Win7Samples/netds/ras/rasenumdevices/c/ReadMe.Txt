RasEnumDevices

RasEnumDevices is a console application that enumerates all the RAS capable device that 
have been configured for Dial Up Networking

FILES
=====

The RasEnumDevices application uses the following files

File                 Description

README.TXT           Readme file for RasEnumDevices application
RASENUMDEVICES.C     C source code
MAKEFILE             Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RASENUMDEVICES


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS NT OR WINDOWS 9x:
-------------------------------------------

To build the application on either Windows NT or Windows 9x you will
need to have the rasapi32.dll installed in the system folder. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program RasEnumDeveices.exe
for Microsoft Windows NT or Windows 9x.


----------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT AND WINDOWS 9x
----------------------------------------------------

To run the application type:

	RasEnumDevices

For Windown 95(pre OSR2) you need to download the DUN 1.2x upgrade.