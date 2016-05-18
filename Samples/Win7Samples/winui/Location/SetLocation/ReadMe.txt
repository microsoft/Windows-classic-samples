==================================
Location API: Set Default Location
Windows 7 Location Platform
==================================
This sample shows how to set the default location using the Location API.
This sample requires Visual Studio to be built.

This sample must be run as administrator for the call to
IDefaultLocation::SetReport to succeed. 

This application prompts the user to specify a civic address that
is used to create a civic address location report, and prints its fields.
It then sets the default location, which can then be viewed in 
Control Panel.

===============================
Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

=====
Files
=====
SetLocation.cpp            The main file
LocationReportObject.h	   Header with declarations of the location report
LocationReportObject.cpp   Implementation of the location report to set
SetLocation.sln            Solution for the project
SetLocation.vcproj         VS Project file
ReadMe.txt		   This ReadMe

============= 
Prerequisites
=============
Windows 7
Visual Studio 2008

========
Building
========
This sample cannot be built in the SDK environment.  Visual Studio must be used.
To build the sample using Visual Studio 2008:
     1. Run the Windows SDK Configuration Tool provided with the SDK to add SDK include directories to Visual Studio
     2. Open Windows Explorer and navigate to the directory.
     3. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type SetLocation.exe at the command line, or double-click the icon for DefaultLocation.exe to launch it from Windows Explorer.

To run from Visual Studio:
     1. Press F5 or Click menu
           Debug->Start Debugging