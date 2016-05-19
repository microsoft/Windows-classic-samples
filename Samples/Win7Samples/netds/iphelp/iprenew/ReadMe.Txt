SUMMARY
======= 
The IPRenew sample illustrates how to programmatically release and renew IPv4 addresses
obtained through DHCP.  This program also demonstrates how to retrieve existing
network adapter configuration information.

IPv4 address assignment from DHCP can be updated using the IpRenewAddress and
IpReleaseAddress functions in IP Helper APIs. These APIs require you to understand adapter index numbers. In Windows, every network adapter has a unique index ID.  
Adapter index IDs can be retrieved using the GetAdaptersInfo() IP Helper API.  
This program features a list option that displays current network adapter configuration
information showing all adapters index numbers associated with their corresponding
network adaptors.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++


FILES
=====
IPRenew.cpp
    This is the main application source file 

IPRenew.sln
    The Visual Studio solution file for building the sample
    
IPRenew.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the IPRenew application. It contains information
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
2. Type msbuild IPRenew.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the IPRenew.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the 
   command prompt or Windows Explorer.
2. Type IPRenew.exe at the command line with the following parameters to launch 
   the sample:

    Iprenew.exe [ -l ] [ -r<index id> ] [ -n<index id> ]

        -l List adapters with corresponding index ID information
        -r Release IP address for adapter index ID
        -n Renew IP address for adapter index ID

