Manipulation and Inertia Sample
================================
Demonstrates how to add Windows Touch support to native Win32 applications using the Manipulation and Inertia API.  The sample implements the essential features of the API to enable translation, rotation and scaling for objects and applying inertia properties to them using a timer.  It also demonstrates how to add basic mouse support in your touch applications.

Minimum Requirements
===============================
Windows 7

Sample Language Implementations
===============================
C++

Files:
=============================================
ManipulationSample.sln
ManipulationSample.vcproj
ManipulationSample.cpp
DrawingObject.cpp
D2DDriver.cpp
CoreObject.cpp
ComTouchDriver.cpp
cmanipulationeventsink.cpp
DrawingObject.h
D2DDriver.h
CoreObject.h
ComTouchDriver.h
cmanipulationeventsink.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the MTManipulationInertia directory.
     2. Type msbuild ManipulationSample.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the MTManipulationInertia directory.
     2. Double-click the icon for the ManipulationSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type ManipulationSample.exe at the command line, or double-click the icon for ManipulationSample.exe to launch it from Windows Explorer.