WPD API Sample
================================
Demonstrates the following using the WPD API:
- Enumerate portable devices
- Enumerate content on a portable device
- Query the capabilities of a portable device
- Read/Write properties for content on a portable device
- Transfer content on/off a portable device
- Register/Unregister for portable device events

Suppported Operating Systems:
=============================
Windows Vista
Windows 7
Windows 8

Sample Language Implementations
===============================
C++

Files:
=============================================
CommonFunctions.h
ContentEnumeration.cpp
ContentProperties.cpp
ContentTransfer.cpp
DeviceCapabilities.cpp
DeviceEnumeration.cpp
DeviceEvents.cpp
ReadMe.txt
stdafx.h
WpdApiSample.cpp
WpdApiSample.sln
WpdApiSample.vcproj

To build the sample using the command prompt:
=============================================
Install Microsoft Visual Studio Developer Express
     1. Open the Command Prompt window and navigate to the WpdApiSample\cpp directory.
     2. Type msbuild WpdApiSample.sln.

To build the sample using Visual Studio:
===========================================================
     1. Open File Explorer and navigate to the WpdApiSample\cpp directory.
     2. Double-click the icon for the WpdApiSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
Note that this sample requires the Microsoft.VC90.CRT redistributable from Microsoft Visual Studio.
     1. Connect a portable device.
     2. Navigate to the directory that contains the new executable, using the command prompt or File Explorer.
     3. Type WpdApiSample.exe at the command line, or double-click the icon for WpdApiSample.exe to launch it from File Explorer.
