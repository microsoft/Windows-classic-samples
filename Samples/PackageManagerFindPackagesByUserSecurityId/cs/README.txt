Find Packages By User Security ID
=================================
This sample demonstrates how to use IPackageManager and IPackage APIs to 
enumerate all packages currently installed for the specified user (by UserSID).

NOTE: This sample must be run in an elevated command prompt if specifying a 
UserSID for a user other than the currently logged on user.

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
FindPackagesByUserSecurityId.cs

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild FindPackagesByUserSecurityIdSample.sln.


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
     2. Navigate to the directory containing FindPackagesByUserSecurityIdSample.exe
     3. Type FindPackagesByUserSecurityIdSample.exe UserSID at the command line.
     		e.g., FindPackagesByUserSecurityIdSample.exe S-1-5-21-923033176-3356060756-4280405249-1000
     
