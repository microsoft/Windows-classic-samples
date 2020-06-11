---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DListView
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Uses a bitmap atlas to create and animate a list of items."
---

# Direct2D List View Sample

This sample shows how to create and use a bitmap atlas to create and animate a list of items. The list view loads the files and directories from the current directory with the icons. 
The directories and files in the list view can be sorted in the alphabetical and reverse alphabetical order. 

This sample is written in C++.

## Files

* DeclareDPIAware.manifest: Declares that the application is DPI-aware.
* ListViewSample.cpp: Contains the application entry point and the implementation of the ListViewAppclass.
* ListViewSample.h: The header file for the ListViewApp class.
* ListViewSample.sln: The sample's solution file. 
* ListViewSample.vcproj: The sample project file. 

## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type msbuild ListViewSample.sln.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## Running the Sample

Use the following keys to sort the list:  

* 'A' key: sorts the list alphabetically. 
* 'Z' Key: sorts the list in reverse alphabetical order. 
* 'D' Key: sorts the list alphabetically with directories first. 

To navigate to a directory, left-click it.
