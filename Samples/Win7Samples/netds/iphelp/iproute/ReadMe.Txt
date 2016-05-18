SUMMARY
======= 
The IPRoute sample illustrates how to the use of IP Helper APIs to
manipulate the IPv4 routing table.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++


FILES
=====
IPRoute.cpp
    This is the main application source file 

IPRoute.h
    This is the application header file 

IPRoute.sln
    The Visual Studio solution file for building the sample
    
IPRoute.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the IPRoute application. It contains information
    about the version of Visual C++ that generated the file, and information about the 
    platforms, configurations, and project features selected with the application
    wizard.

Makefile
    The Makefile for use the with the NMAKE command for building the sample

Readme.txt        The Readme file


PLATFORMS SUPORTED
==================
Windows 98 or later.


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild IPRoute.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the IPRoute.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the 
   command prompt or Windows Explorer.
2. Type IPRoute.exe at the command line with the following parameters to launch 
   the sample:

IpRoute -p                                               ...Prints route table.
IpRoute -a destination netmask gateway interface [metric]...Adds a route.
IpRoute -d destination                                   ...Deletes routes to
                                                            destination.

destination  Specifies the destination host.

netmask      Specifies a subnet mask value to be associated
             with this route entry.

gateway      Specifies gateway.

interface    Specifies the interface ip.

metric       The cost for this destination


Diagnostic Notes:
=================
Invalid MASK generates an error, that is when (DEST & MASK) != DEST.
Example> IpRoute -a 157.0.0.0 155.0.0.0 157.55.80.1 157.55.80.9
         IpRoute: Invalid Mask 155.0.0.0

Examples:

> IpRoute -p
> IpRoute -a 157.0.0.0    255.0.0.0 157.55.80.1  157.55.80.9           1
             ^destination ^mask     ^gateway     ^existing interface   ^metric
> IpRoute -p
> IpRoute -d 157.0.0.0
> IpRoute -p

