SUMMARY
======= 
The IPStat sample illustrates how to the use of IP Helper APIs to
Shows IPv4 connections for a protocol. By default, statistics are shown for
IP, ICMP, TCP and UDP. 


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++


FILES
=====
IPStat.cpp
    This is the main application source file 

IPStat.h
    This is the application header file 

IPStat.sln
    The Visual Studio solution file for building the sample
    
IPStat.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the IPStat application. It contains information
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
2. Type msbuild IPStat.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the IPStat.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the 
   command prompt or Windows Explorer.
2. Type IPStat.exe at the command line with the following parameters to launch 
   the sample:

IpStat -p proto      Shows connections for the protocol specified
                     by proto, proto may be tcp or udp.
IpStat -s [-p proto] Displays per-protocol statistics.
                     By default, statistics are shown for
                     IP, ICMP, TCP and UDP; the -p option
                     may be used to specify a subset of the default.
Examples:

> IpStat -p tcp
> IpStat  -s
