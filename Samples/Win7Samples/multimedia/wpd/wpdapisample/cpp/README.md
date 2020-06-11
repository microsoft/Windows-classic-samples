---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: WPD API sample
urlFragment: wpd-sample
description: Demonstrates how to use the WPD API. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# WPD API sample

Demonstrates the following using the WPD API:
- Enumerate portable devices
- Enumerate content on a portable device
- Query the capabilities of a portable device
- Read/Write properties for content on a portable device
- Transfer content on/off a portable device
- Register/Unregister for portable device events

## Suppported operating systems:

Windows Vista
Windows 7

## Sample language implementations

C++

## Files:

- *CommonFunctions.h*
- *ContentEnumeration.cpp*
- *ContentProperties.cpp*
- *ContentTransfer.cpp*
- *DeviceCapabilities.cpp*
- *DeviceEnumeration.cpp*
- *DeviceEvents.cpp*
- *README.md*
- *stdafx.cpp*
- *stdafx.h*
- *WpdApiSample.cpp*
- *WpdApiSample.sln*
- *WpdApiSample.vcproj*

## To build the sample using the command prompt

Note that this sample uses ATL. This means you must install Microsoft Visual Studio to compile this sample.
1. Open the Command Prompt window and navigate to the *WpdApiSample\cpp* directory.
1. Type **msbuild WpdApiSample.sln**.

To build the sample using Visual Studio (preferred method)

1. Open Windows Explorer and navigate to the *WpdApiSample\cpp* directory.
1. Double-click the icon for the *WpdApiSample.sln* file to open the file in Visual Studio.
1. In the **Build** menu, select **Build Solution**. The application will be built in the default *Debug* or *Release* directory.

## To run the sample

Note that this sample requires the Microsoft.VC90.CRT and Microsoft.VC90.ATL redistributables from Microsoft Visual Studio.
1. Connect a portable device.
1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
1. Type **WpdApiSample.exe** at the command line, or double-click the icon for *WpdApiSample.exe* to launch it from Windows Explorer.
