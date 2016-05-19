SUMMARY
======= 
The IPArp sample illustrates how to use the IP Helper functions to display and
manipulate entries in the IPv4 ARP table on the local computer. 


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++


FILES
=====
IPArp.cpp
    This is the application source file

IPArp.
    This is the header file for the application

IPArp.sln
    The Visual Studio solution file for building the sample
    
IPArp.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the IPArp application. It contains information
    about the version of Visual C++ that generated the file, and information about the 
    platforms, configurations, and project features selected with the application
    wizard.

Makefile
    The Makefile for use the with the NMAKE command for building the sample

Readme.txt        The Readme file


PLATFORMS SUPORTED
==================
Windows 2000 or later.


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild IPArp.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the IPArp.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type IPArp .exe at the command line with various command-line options to launch the sample.
