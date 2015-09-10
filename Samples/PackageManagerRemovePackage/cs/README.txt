Remove Package Sample
====================
This sample demonstrates how to use IPackageManager to remove a package.

Prerequisites
=============
This sample requires Windows 8.1+.
This sample requires Visual Studio 12 Ultimate Developer Preview.
This sample requires the Windows Runtime Software Development Kit.

Sample Language Implementations
===============================
C#

Files:
======
RemovePackageSample.cs

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild RemovePackageSample.sln.


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
     2. Navigate to the directory containing RemovePackageSample.exe 
     2. Type RemovePackageSample.exe <package-fullname> at the command line. 
     For example, RemovePackageSample.exe testapp_1.0.0.0_neutral_en-us_ab1c2d3efghij
