==============================
CustomAccServer Sample
Microsoft Active Accessibility
==============================
This sample shows how to implement a custom control with a Microsoft Active Accessibility (MSAA) server.

The control itself has been kept simple. It does not support scrolling and therefore an arbitrary 
limit has been set on the number of items it can contain. For convenience, list items are stored 
in a deque (from the Standard Template Library).
 
The accessible object consists of the root element (a list box) and its children (the list items.)

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
AccServer.cpp				Implementation of the accessible object
AccServer.h				Declarations for the accessible object
AccServer.ico				Application icon
AccServer.rc				Application resource file
AccServer.vcproj			VS project file
CustomAccServer.sln			VS solution file
CustomControl.cpp			Implementation of the custom list control
CustomControl.h				Declarations for the custom list control
EntryPoint.cpp				Main application entry point
ReadMe.txt       			This ReadMe
resource.h				VS resource file
small.ico				Small icon
stdafx.h                                Precompiled header

==================== 
Minimum Requirements
====================
Windows XP, Windows Server 2003
Visual Studio 2008

========
Building
========
To build the sample using Visual Studio 2008:
     1. Run the Windows SDK Configuration Tool provided with the SDK to add SDK include directories to Visual Studio.
     2. Open Windows Explorer and navigate to the project directory.
     3. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To build the sample from the command line, see Building Samples in the Windows SDK release notes at the following location:
	%Program Files%\Microsoft SDKs\Windows\v7.0\ReleaseNotes.htm

=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type AccServer.exe at the command line, or double-click the icon for AccServer.exe 
	to launch it from Windows Explorer.

To run from Visual Studio, press F5 or click Debug->Start Debugging.