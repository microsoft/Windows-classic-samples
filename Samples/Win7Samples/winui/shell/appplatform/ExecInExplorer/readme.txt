Execute in Explorer Sample
================================
Demonstrates how to perform a ShellExecute in the Explorer Process. This is most useful when you are an elevated process that you want to run in an unelevated. The Windows Explorer runs unelevated most of the time so applications can use this code to take advantage of that.

Sample Language Implementations
===============================
C++

Files:
=============================================
ExecInExplorer.cpp
ExecInExplorer.sln
ExecInExplorer.vcproj


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the ExecInExplorer directory.
     2. Type msbuild ExecInExplorer.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the ExecInExplorer directory.
     2. Double-click the icon for the ExecInExplorer.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run ExecInExplorer.exe
