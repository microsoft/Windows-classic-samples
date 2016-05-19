CreateProcess Verb Sample
================================
Demonstrates how implement a shell verb using the CreateProcess method CreateProcess based verbs depend on running a executable and passing it a command line argument. This method is not as powerful as the DropTarget and ExecuteCommand methods but it does achieve the desirable out of process behavior.

Sample Language Implementations
===============================
 C++

Files:
=============================================
CreateProcessVerb.cpp
CreateProcessVerb.sln
CreateProcessVerb.vcproj
RegisterExtension.cpp
RegisterExtension.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the CreateProcessVerb directory.
     2. Type msbuild CreateProcessVerb.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the CreateProcessVerb directory.
     2. Double-click the icon for the CreateProcessVerb.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run CreateProccessVerb.exe
     3. Follow the instructions in the displayed dialog
