Execute Command Verb Sample
================================
Demonstrates how implement a shell verb using the ExecuteCommand method. This method is preferred for verb implementations as it provides the most flexibility, it is simple, and supports out of process activation. This sample implements a standalone local server COM object but it is expected that the verb implementation will be integrated into existing applications. to do that have your main application object register a class factory for itself and have that object implement IDropTarget for the verbs of your application. Note that COM will launch your application if it is not already running and will connect to an already running instance of your application if it is already running. These are features of the COM based verb implementation methods.


Sample Language Implementations
===============================
C++

Files:
=============================================
ExecuteCommand.sln
ExecuteCommand.vcproj
ExecuteCommandVerb.cpp
RegisterExtension.cpp
RegisterExtension.h
ShellHelpers.h
VerbHelpers.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the ExecuteCommandVerb directory.
     2. Type msbuild ExecuteCommand.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the ExecuteCommandVerb directory.
     2. Double-click the icon for the ExecuteCommand.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run ExecuteCommand.exe
     3. Follow the instructions in the displayed dialog
