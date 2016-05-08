Multi-touch Manipulation Application
====================================
Demonstrates how to setup and handle manipulation multi-touch events in a Win32 application.

This application will initially draw rectangle in the center of client area (including diagonals). The user can manipulate the rectangle using his/her fingers. The available commands are:
- rectangle stretch 
    By putting two fingers on the screen and modifying distance between 
    them by moving fingers in the opposite directions or towards each
    other the user can zoom in/out this rectangle.
- panning
    By touching the screen with two fingers and moving them in the same 
    direction the user can move the rectangle. Also it's possible to pan
    object by using single finger (SFP - single finger panning)
- rotate
    By putting one finger in the center of the rotation and then rotating 
    the other finger around it the user can rotate the rectangle
Demonstrates: IManipulationProcessor, _IManipulationEvents

Sample Language Implementations
===============================
C++

Files
=====
MTManipulation.sln
MTManipulation.vcproj
MTManipulation.cpp
MTManipulation.rc
CDrawingObject.cpp
CDrawingObject.h
CManipulationEventSink.cpp
CManipulationEventSink.h
Resource.h
readme.txt

To build the sample using the command prompt
============================================
     1. Copy sample directory outside Program Files folder.
     2. Open the Command Prompt window and navigate to the copied sample directory.
     3. Type vcbuild MTManipulation.sln. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To build the sample using Visual Studio
=======================================
     1. Copy sample directory outside Program Files folder.
     2. Open Windows Explorer and navigate to the copied sample directory.
     3. Double-click the icon for the MTManipulation.sln file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Win32 or \x64, \Debug or \Release directory.

To run the sample
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MTManipulation.exe at the command line, or double-click the icon for MTManipulation.exe to launch it from Windows Explorer.     
