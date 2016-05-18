Player Verb Sample
================================
Demonstrates how to create a verb that operates on shell items and containers to play items or add items to a queue. This sample is particularly useful for media applications. 


Sample Language Implementations
===============================
C++


Files:
=============================================
ApplicationVerb.h
DragDropHelpers.h
PlayerVerbSample.cpp
PlayerVerbSample.rc
PlayerVerbSample.sln
PlayerVerbSample.vcproj
RegisterExtension.cpp
RegisterExtension.h
resource.h
ShellHelpers.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the PlayerVerbSample directory.
     2. Type msbuild PlayerVerbSample.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the PlayerVerbSample directory.
     2. Double-click the icon for the PlayerVerbSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type PlayerVerbSample.exe at the command line, or double-click the icon for PlayerVerbSample.exe to launch it from Windows Explorer. Select "Register Verbs" in the "Player Verb Sample" dialog.
     3. Right-click on picture items (file, folder or stack) in Explorer to add them to a queue or play them in the sample application. Use music items when you change the sample to work with music.

