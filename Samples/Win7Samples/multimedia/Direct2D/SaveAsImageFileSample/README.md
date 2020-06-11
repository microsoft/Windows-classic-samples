---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DSaveAsImageFile
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Uses a WIC bitmap render target to generate an image and save it as a file."
---
# Direct2D Save As Image Sample

Uses a WIC bitmap render target to generate an image and save it as a file.

This sample is written in C++.
 
## Files

* SaveAsImageFileSample.cpp: Contains the application entry point.
* SaveAsImageFileSample.sln: The sample's solution file.
* SaveAsImageFileSample.vcproj: The sample project file.

When you run the sample, it produces an image file named output.png.
 
## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type msbuild SaveAsImageFileSample.sln.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
