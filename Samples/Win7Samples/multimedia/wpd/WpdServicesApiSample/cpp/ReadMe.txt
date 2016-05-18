WPD Services API Sample
================================
Demonstrates the following using the WPD API:
- Enumerate portable device services
- Enumerate content on a portable device service
- Query the capabilities of a portable device service
- Read/Write properties for content on a portable device service
- Invoke methods on a portable device service

Sample Language Implementations
===============================
C++

Suppported Operating Systems:
=============================
Windows 7

Files:
=============================================
CommonFunctions.h
ContentEnumeration.cpp
ContentProperties.cpp
ReadMe.txt
ServiceCapabilities.cpp
ServiceEnumeration.cpp
ServiceMethods.cpp
stdafx.cpp
stdafx.h
WpdServicesApiSample.cpp
WpdServicesApiSample.sln
WpdServicesApiSample.vcproj

To build the sample using the command prompt:
=============================================
Note that this sample uses ATL. This means you must install Microsoft Visual Studio to compile this sample.
     1. Open the Command Prompt window and navigate to the WpdServicesApiSample\cpp directory.
     2. Type msbuild WpdServicesApiSample.sln.

To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the WpdServicesApiSample\cpp directory.
     2. Double-click the icon for the WpdServicesApiSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
Note that this sample requires the Microsoft.VC90.CRT and Microsoft.VC90.ATL redistributables from Microsoft Visual Studio.
     1. Connect a portable device that supports a Microsoft Contacts Device Service.
     2. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     3. Type WpdServicesApiSample.exe at the command line, or double-click the icon for WpdServicesApiSample.exe to launch it from Windows Explorer.
