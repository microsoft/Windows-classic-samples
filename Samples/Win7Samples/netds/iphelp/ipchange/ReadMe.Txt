SUMMARY
======= 
The IPChange sample illustrates how to use IP Helper functions to programmatically change an IP address for a specific network adapter on your machine.  This program also demonstrates how to retrieve existing network adapter IP configuration information.


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++


FILES
=====
IPChange.cpp
    This is the main application source file 

IPChange.sln
    The Visual Studio solution file for building the sample
    
IPChange.vcproj
    This is the main project file for VC++ projects generated using an application
    wizard. 
    This project file builds the EnableRouter application. It contains information
    about the version of Visual C++ that generated the file, and information about the 
    platforms, configurations, and project features selected with the application
    wizard.

Makefile
    The Makefile for use the with the NMAKE command for building the sample

Readme.txt        The Readme file


PLATFORMS SUPORTED
==================
Windows 2000 and later.


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild IPChange.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the IPChange.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the command prompt.
2. Type IPChange.exe at the command line with various command-line options to 
   launch the sample.
