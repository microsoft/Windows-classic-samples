Shell Library Backup Sample
=====================
Demonstrates how to enumerate libraries as containers. ShellLibraryBackup is a fictional backup 
application that supports picking libraries via the CFD and backing up libraries using the shell 
namespace walker. Libraries are the new means of storage for user files in Windows 7. The Documents, 
Pictures, Music and Videos libraries provide a superset of functionality to users and they look a 
little different when discovered programmatically. 

Sample Language Implementations
===============================
C++

Files:
=============================================
ShellLibraryBackup.cpp
ShellLibraryBackup.h
ShellLibraryBackup.rc
ShellLibraryBackup.sln
ShellLibraryBackup.vcproj
resource.h
targetver.h

Prerequisites:
=============================================
     This sample must be run on Windows 7.


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the ShellLibraryBackup directory.
     2. Type msbuild ShellLibraryBackup.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the LibraryBackup directory.
     2. Double-click the icon for the LibraryBackup.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type ShellLibraryBackup.exe at the command line, or double-click the icon for ShellLibraryBackup.exe to launch it from Windows Explorer.