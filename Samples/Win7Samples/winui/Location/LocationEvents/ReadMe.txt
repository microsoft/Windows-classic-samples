=========================
Location API Asynchronous Access
Windows 7 Location Platform
=========================
This sample shows how to access the Location Platform in an asynchronous manner. This sample requires Visual Studio to be built.

===============================
Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

=====
Files
=====
LocationCallback.h     Header file for callback class
LocationCallback.cpp   Implementation for callback class
LocationEvents.cpp     The main file
LocationEvents.sln     Solution for the project
LocationEvents.vcproj  VS Project file
ReadMe.txt             This ReadMe

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
     2. Type LocationEvents.exe at the command line, or double-click the icon for LocationEvents.exe to launch it from Windows Explorer.

To run from Visual Studio:
     1. Press F5 or Click menu
           Debug->Start Debugging