Find Packages with Package Types Sample
=======================================
This sample demonstrates how to use IPackageManager2 and IPackage2 APIs to 
enumerate every package currently installed on the machine. Additionally, the
sample shows how to use the packageTypes parameter to filter the search to
a specific type of packages.  

NOTE: This sample must be run in an elevated command prompt.

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
FindPackagesWithPackageTypesSample.cpp

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild FindPackagesSample.sln.


To build the sample using Visual Studio 12 Ultimate Developer Preview (preferred method):
================================================
     1. Open File Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in 
     Visual Studio.
     3. In the Build menu, select Build Solution. The application will be 
     built in the default \Debug or \Release directory.


To Run the sample:
==================
     1. Open an elevated command prompt.
     2. Navigate to the directory containing FindPackagesWithPackageTypesSample.exe
     2. Type FindPackagesWithPackageTypesSample.exe at the command line.
