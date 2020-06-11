---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DSimplePathAnimation
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Demonstrates how to create a simple path animation with Direct2D."
---

# Direct2D Simple Path Animation Sample

Demonstrates how to create a simple path animation with Direct2D.

This sample is written in C++. 

## Files

* Animation.h: Defines animation helpers.
* DeclareDPIAware.manifest: Declares that the sample is DPI aware. 
* SimplePathAnimationSample.cpp: Contains the application entry point and the implementation of the DemoApp class.
* SimplePathAnimationSample.h: The header file for the DemoApp class.
* SimplePathAnimationSample.sln: The sample's solution file.
* SimplePathAnimationSample.vcproj: The sample project file.
 
## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 


## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type msbuild SimplePathAnimationSample.sln.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
