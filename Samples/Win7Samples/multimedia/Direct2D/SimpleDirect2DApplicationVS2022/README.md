---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Simple Direct2D Application
urlFragment: simple-direct-2d-application
description: Draws shapes, text, and images with Direct2D.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Simple Direct2D Application

Draws shapes, text, and images with Direct2D.

This sample is written in C++.

## Files

* **SimpleDirect2dApplication.cpp**: Contains the application entry point and the implementation of the `DemoApp` class.
* **SimpleDirect2dApplication.h**: The header file for the DemoApp class.
* **SimpleDirect2dApplication.sln**: The sample's solution file.
* **SimpleDirect2dApplication.vcproj**: The sample project file.

## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type **msbuild SimpleDirect2dApplication.sln**.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the *.sln* (solution) file to open the file in Visual Studio.
3. In the **Build** menu, select **Build Solution**. The application will be built in the default *\Debug* or *\Release* directory.
