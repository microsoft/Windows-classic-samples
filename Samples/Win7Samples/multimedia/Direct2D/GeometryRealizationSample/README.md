---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: Direct2DGeometryRealization
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Uses opacity masks and A8 targets to enhance performance for anti-aliased geometries."
---
# Direct2D Geometry Realization Sample

This sample shows how to use opacity masks and A8 targets to enhance performance for anti-aliased geometries. 
It also shows how to use meshes to enhance performance for aliased geometries. 

This sample is written in C++.

## Files

* DeclareDPIAware.manifest: Specifies that the sample application is DPI-aware.
* GeometryRealization.cpp: Implements the IGeometryRealization interface.
* GeometryRealization.h: Defines the IGeometryRealization interface and related types.
* GeometryRealizationSample.cpp: Implements the DemoApp class, which creates a window and demonstrates the IGeometryRealization interface.
* GeometryRealizationSample.h: Defines the the DemoApp class.
* GeometryRealizationSample.sln: The sample's solution file.
* GeometryRealizationSample.vcproj: The sample project file.
* RingBuffer.h: The header file for the RingBuffer class. RingBuffer works like a standard array, except that when it fills up, data at the beginning is overwritten.
* stdafx.h: Defines standard system include files, or project specific include files that are used frequently, but are changed infrequently.

## Prerequisites

* Microsoft Windows 7
* Windows Software Development Kit (SDK) for Windows 7 and .NET Framework 3.5 Service Pack 1 

## Building the Sample

To build the sample using the command prompt:

1. Open the Command Prompt window and navigate to the sample directory.
2. Type msbuild GeometryRealizationSample.sln.

To build the sample using Visual Studio 2008 (preferred method):

1. Open Windows Explorer and navigate to the sample directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

## Running the Sample

This sample uses the following controls: 

* Up Arrow: Increases the number of primitives rendered 
* Down Arrow: Decreases the number of primitives rendered 
* Spacebar: Pauses/resumes the animation. 
* Mouse Wheel: Zooms in and out. 
* 'T' key: Toggles between hardware and software rendering. 
* 'R' key: Toggles between rendering geometry with and without realizations. 
* 'A' key: Toggles between rendering modes. 
