DragDrop Visuals Sample
================================
Demonstrates how to use the shell drag drop services to get the presentation features that shell drag drop supports for both targets and sources. 

This includes:
 1) drop targets rendering the drag image
 2) drop target provided drop tips
 3) drag source populating the drag image information when using a custom data object
 4) drag source enable drop tips
 5) use the shell provided IDropSource implementation by calling SHDoDragDrop(). This handles many of the edge cases for you dealing with different types of targets

Sample Language Implementations
===============================
 C++

Files:
=============================================
DataObject.cpp
DataObject.h
DragDropHelpers.cpp
DragDropHelpers.h
DragDropVisuals.cpp
DragDropVisuals.rc
DragDropVisuals.sln
DragDropVisuals.vcproj
resource.h
ShellHelpers.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the DragDropVisuals directory.
     2. Type msbuild DragDropVisuals.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the DragDropVisuals directory.
     2. Double-click the icon for the DragDropVisuals.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run DragDropVisuals.exe
     3. Follow the instructions in the displayed program
