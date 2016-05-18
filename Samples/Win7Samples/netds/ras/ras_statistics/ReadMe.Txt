RasGetConnectionStatistics

RasGetConnectionStatistics is a console application that displays the statistics of the current active RAS connection.

FILES
=====

The RasGetConnectionStatistics application uses the following files

File            		Description

README.TXT      		Readme file for RasGetConnectionStatistics application
RASGETCONNECTIONSTATISTICS.C    C source code
MAKEFILE        		Nmake file for Windows 2000


RAS API's USED
==============

RASENUMCONNECTIONS
RASGETCONNECTIONSTATISTICS


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS 2000:
-------------------------------------------

To build the application on Windows 2000 you will need to have the rasapi32.dll 
installed in the system folder. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program RasGetConnectionStatistics.exe
for Microsoft Windows 2000.


--------------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS 2000
--------------------------------------------------------

To run the application type:

    RasGetConnectionStatistics


-------------------------
ADDITIONAL INFORMATION
-------------------------

The sample assumes that there is only one current active RAS connection on the system.
