SUMMARY
======= 
The netinfo sample illustrates how to the use the IP Helper APIs introduced on
Windows Vista to display/change address and interface info for IPv4 and IPv6.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C


FILES
=====
netinfo.c
    This is the main application source file 

netinfo.h
    This is the application header file 

netinfo.sln
    The Visual Studio solution file for building the sample
    
netinfo.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the netinfo application. It contains information
    about the version of Visual C++ that generated the file, and information about the 
    platforms, configurations, and project features selected with the application
    wizard.

Makefile
    The Makefile for use the with the NMAKE command for building the sample

Readme.txt        The Readme file


PLATFORMS SUPORTED
==================
Windows Vista and later.


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild netinfo.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the netinfo.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the 
   command prompt or Windows Explorer.
2. Type netinfo.exe at the command line to launch the sample:

