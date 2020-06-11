---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DTextAnimation
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Demonstrates scaling, transforming, and rotating of text using Direct2D."
---
# Direct2D Text Animation Sample

Demonstrates scaling, transforming, and rotating of text using Direct2D.

This sample is written in C++.
 
## Files

* DeclareDPIAWare.manifest: Specifies that the application is DPI aware.
* RingBuffer.h: The header file for the RingBuffer class. RingBuffer works like a standard array, except that when it fills up, data at the beginning of the array is overwritten.
* TextAnimationSample.cpp: Contains the application entry point and the implementation of the DemoApp class.
* TextAnimationSample.h: The header file for the DemoApp class.
* TextAnimationSample.sln: The sample's solution file.
* TextAnimationSample.vcproj: The sample project file.

## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type msbuild TextAnimationSample.sln.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## Running the Sample

This sample uses the following controls:

Animation controls:
* "t" : Toggles the translation animation off and on.
* "r" : Toggles the rotation animation off and on.
"s" : Toggles the scale animation off and on.

Rendering mode controls:
* "1" : Uses the default text rendering mode.
* "2" : Uses the outline text rendering mode.
* "3" : Uses an A8 target as an opacity mask to render the text.
