Custom Jump List Sample
================================
Demonstrates how to create a custom Jump List for an application, including adding a custom category and Tasks.

Sample Language Implementations
===============================
C++

Files:
=============================================
CustomJumpListSample.cpp
CustomJumpListSample.ico
CustomJumpListSample.rc
FileRegistrations.h
resource.h


Prerequisites:
=============================================
     None


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the CustomJumpList directory.
     2. Type msbuild CustomJumpListSample.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the CustomJumpList directory.
     2. Double-click the icon for the CustomJumpListSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type CustomJumpListSample.exe at the command line, or double-click the icon for CustomJumpListSample.exe to launch it from Windows Explorer.
     3. This sample must be run as an administrator the first time it is run so it can install the necessary file type registrations.  After the file types have been registered, the sample may run as a standard user.
     4. Select options from the menu in the sample application to see how they affect the application's Jump List in the taskbar.
