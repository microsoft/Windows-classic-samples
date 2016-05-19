RasEnumConnections

RasEnumConnections is a console application that enumerates all active RAS connections

FILES
=====

The RasEnumConnections application uses the following files

File                 	Description

README.TXT           	Readme file for RasEnumConnections application
RasEnumConnections.C    C source code
MAKEFILE             	Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RasEnumConnections


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS NT OR WINDOWS 9x:
-------------------------------------------

To build the application on either Windows NT or Windows 9x you will
need to have the rasapi32.dll installed in the system folder. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program RasEnumConnections.exe
for Microsoft Windows NT or Windows 9x.


----------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT AND WINDOWS 9x
----------------------------------------------------

To run the application type:

	RasEnumConnections