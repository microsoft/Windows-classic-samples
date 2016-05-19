SUMMARY
======= 
The EnableRouter sample illustrates how to use the EnableRouter and UnEnableRouter IP Helper functions to enable and disable IPv4 forwarding on the local computer. 


Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C


FILES
=====
enablerouter.c
    This is the main application source file 

enablerouter.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses. This file can be directly edited in Microsoft Visual C++. 

enablerouter.sln
    The Visual Studio solution file for building the sample
    
enablerouter.vcproj
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
Windows 98 or later.


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the directory.
2. Type msbuild enablerouter.sln (solution file)
  or
1. Open the Command Prompt window and navigate to the directory.
2. Type NMAKE
 

To build the sample using Visual Studio 2005 (preferred method):
================================================
1. Open Windows Explorer and navigate to the  directory.
2. Double-click the icon for the enablerouter.sln (solution) file to open the file 
   in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the
   default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type enablerouter.exe at the command line to launch the sample.
