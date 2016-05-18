RasEnumEntries

RasEnumEntries is a console application that enumerates all the phonebook entries from 
the default phonebook.

FILES
=====

The RasEnumEntries application uses the following files

File                 Description

README.TXT           Readme file for RasEnumEntries application
RASENUMENTRIES.C     C source code
MAKEFILE             Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RASENUMENTRIES


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS NT OR WINDOWS 9x:
-------------------------------------------

To build the application on either Windows NT or Windows 9x you will
need to have the rasapi32.dll installed in the system folder. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program RasEnumEntries.exe
for Microsoft Windows NT or Windows 9x.


----------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT AND WINDOWS 9x
----------------------------------------------------

To run the application type:

	RasEnumEntries
