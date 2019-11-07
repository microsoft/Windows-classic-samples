Find Packages Sample
====================
This sample demonstrates how to use IPackageManager and IPackage APIs to 
enumerate every package currently provisioned for all users on the machine.

NOTE: This sample must be run in an elevated command prompt.

Prerequisites
=============
This sample requires Windows 10, build 18917 or later.
This sample requires Visual Studio 2019 Preview or later.
This sample requires the Windows Runtime Software Development Kit.

Sample Language Implementations
===============================
C#

Files:
======
FindProvisionedPackagesSample.cs

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild FindProvisionedPackagesSample.sln.


To build the sample using Visual Studio 2019 Preview (preferred method):
================================================
     1. Open File Explorer and navigate to the directory.
     2. Double-click the icon for the .sln (solution) file to open the file in 
     Visual Studio.
     3. In the Build menu, select Build Solution. The application will be 
     built in the default \Debug or \Release directory.


To run the sample:
==================
     1. Open an elevated command prompt.
     2. Navigate to the directory containing FindProvisionedPackagesSample.exe
     2. Type FindProvisionedPackagesSample.exe at the command line.
