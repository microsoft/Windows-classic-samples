Bluetooth Connection (BthCxn) Sample

Sample to demonstrating making a Bluetooth connection over RFCOMM using Winsock version 2.2.

Files

bthcxn – folder containing the Bluetooth Connection Sample's source code
  bthcxn.cpp - source code for the Bluetooth Connection Sample
  bthcxn.rc - resource file with sample's versioning information
  bthcxn.sln - sample's Visual Studio solution file
  bthcxn.vcproj - sample's Visual Studio project file
 
Prerequisites

* Supported Bluetooth radio hardware (2)
* Minimum Microsoft Windows XP SP2

Building the Sample

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild bthcxn.sln


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

