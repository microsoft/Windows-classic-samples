Find Packages By Name and Publisher Sample
==========================================
This sample demonstrates how to use IPackageManager and IPackage APIs to 
enumerate all packages with the specified name and publisher currently 
installed on the machine. This sample also demonstrates how to enumerate every 
user that has a given package installed.

NOTE: This sample must be run in an elevated command prompt.

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
FindPackagesbyNameAndPublisherSample.cs

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild FindPackagesbyNameAndPublisherSample.sln.


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
     2. Navigate to the directory containing FindPackagesbyNameAndPublisherSample.exe
     3. Type FindPackagesbyNameAndPublisherSample.exe PackageName PublisherName at the command 
     line. Arguments containing whitespace must be surrounded by 
     double quotes (").
     		e.g., FindPackagesbyNameAndPublisherSample.exe microsoft.help "CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US"
     
