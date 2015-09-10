Add Package Sample
====================
This sample demonstrates how to use IPackageManager to install a package.

Prerequisites
=============
This sample requires Windows 8.1+.
This sample requires Visual Studio 12 Ultimate Developer Preview.
This sample requires the Windows Runtime Software Development Kit.

Sample Language Implementations
===============================
C++

Files:
======
AddPackageSample.cpp

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild AddPackageSample.sln.


To build the sample using Visual Studio 12 Ultimate Developer Preview (preferred method):
================================================
     1. Open File Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in 
     Visual Studio.
     3. In the Build menu, select Build Solution. The application will be 
     built in the default \Debug or \Release directory.


To Run the sample:
==================
     1. Open a command prompt.
     2. Navigate to the directory containing AddPackageSample.exe 
     2. Type AddPackageSample.exe "<uri-of-package>" at the command line. 
     For example, AddPackageSample.exe "file://C|/users/testuser/desktop/testpackage.appx"
