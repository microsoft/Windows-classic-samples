RasDialAsync

RasDialAsync is a console application that allows you to make a an asynchronous RAS 
connection.

FILES
=====

The RasDial application uses the following files

File              Description

README.TXT        Readme file for RasDialAsync application
RASDIALASYNC.C    C source code
MAKEFILE          Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RASDIAL
RASGETCONNECTSTATUS
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

The above commands build the executable program RasDialAsync.exe
for Microsoft Windows NT or Windows 9x.


-------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT 
-------------------------------------

To run the application type:

	RasDialAsync -e [entry name] -p [phone number] -u [username] -z [password] -d [domain]

-------------------------------------
RUNNING THE APPLICATION ON WINDOWS 9x
-------------------------------------

If you wish to run the application on Windows 95, you will need to have at least ONE 
entry in the phonebook.

To run the application type:

	RasDialAsync -e [entry name] -p [phone number] -u [username] -z [password] -d [domain]
