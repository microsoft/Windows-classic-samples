DropTarget Verb Sample
================================
Demonstrates how implement a shell verb using the DropTarget method. This method is preferred for verb implementations that need to work on Windows XP due to it's flexibility, simplicity and support for out of process activation. This sample implements a standalone local server COM object but it is expected that the verb implementation will be integreated into existing applications. To do that have your main application object register a class factory for itself and have that object implement IDropTarget for the verbs of your application. Note that COM will launch your application if it is not already running and will connect to an already running instance of your application if it is already running. These are features of the COM based verb implementation methods.

Sample Language Implementations
===============================
C++

Files:
=============================================
DropTargetVerb.vcproj
DropTargetVerb.cpp
DropTargetVerb.sln
RegisterExtension.cpp
RegisterExtension.h
ShellHelpers.h
VerbHelpers.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the DropTargetVerb directory.
     2. Type msbuild DropTargetVerb.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the DropTargetVerb directory.
     2. Double-click the icon for the DropTargetVerb.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run DropTargetVerb.exe
     3. Follow the instructions in the displayed dialog
