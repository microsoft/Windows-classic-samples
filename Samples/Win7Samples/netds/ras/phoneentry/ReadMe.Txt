PhoneEntry

PhoneEntry is GUI based application that allows you to create new phonebook entries or 
modify existing ones.

FILES
=====

The PhoneEntry application uses the following files

File           Description

README.TXT     Readme file for the PhoneEntry application
PHONEENTRY.C   C source code
PHONEENTRY.RC  Resource file
RESOURCE.H     Header file needed for the resource file
MAKEFILE       Nmake file for Windows NT or Windows 9x


RAS API's USED
==============

RASSETENTRYPROPERTIES
RASGETENTRYPROPERTIES
RASENUMENTRIES
RASENUMDEVICES
RASGETCOUNTRYINFO
RASVALIDATEENTRYNAME


-------------------------------------------
BUILDING THE APPLICATION FOR
MICROSOFT WINDOWS NT OR WINDOWS 9x:
-------------------------------------------

To build the application on either Windows NT or Windows 9x you will
need to have the rasapi32.dll installed. 

In Windows NT and Windows98 rasapi32.dll comes with the system but you need to install 
Dial Up Networking (DUN). 

On Windows 95 you will need to install DUN and then download the at least DUN 1.2b upgrade. 

Build the application:

  nmake cleanall
  nmake

The above commands build the executable program PhoneEntry.exe
for Microsoft Windows NT or Windows 9x.


-------------------------------------
RUNNING THE APPLICATION ON WINDOWS NT
-------------------------------------

To run the application type:

   PhoneEntry


---------------------
RUNNING ON WINDOWS 9x
---------------------

If you wish to run the application on Windows 95, you will need to have a phonebook 
with at least ONE entry.

To run the application type:

  PhoneEntry
