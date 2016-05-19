RasDial

SDKRasDial is a console application that allows you to make a synchronous (or silent) 
RAS connection.

FILES
=====

The RasDial application uses the following files

File            Description

README.TXT      Readme file for RasDial application
RASDIAL.C       C source code
MAKEFILE        Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RASDIAL
RASGETCONNECTSTATUS
RASGETPROJECTIONINFO
RASHANGUP


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS NT OR WINDOWS 9x:
-------------------------------------------

To build the application on either Windows NT or Windows 9x you will
need to have the rasapi32.dll installed in the system folder. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program RasDial.exe
for Microsoft Windows NT or Windows 9x.


-------------------------------------------
OVERVIEW
-------------------------------------------

The application dials a specified entry name using RasDial API. Once a connection 
is established and verified using RasGetConnectStatus, the application outputs 
the server's and client's IP addresses by using RasGetProjectionInfo. The user 
can disconnect by pressing 'ENTER'. This is achieved by calling RasHangUp. For 
demonstration purposes we wait for 10 seconds checking if the connection was 
actually hung up. This is done agian by calling RasGetConnectStatus and checking 
for ERROR_INVALID_HANDLE. 


--------------------------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT AND WINDOWS 9x
--------------------------------------------------------

To run the application type:

    SDKRasDial -e [entry name] -p [phone number] -u [username] -z [password] -d [domain]

You will need to have a phonebook entry with the entry name in the default phonebook



