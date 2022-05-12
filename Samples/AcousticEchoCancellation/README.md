---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Acoustic echo cancellation
urlFragment: AcousticEchoCancellation
description: Set the reference endpoint id for acoustic echo cancellation.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Acoustic Echo Cancellation Sample

This sample demonstrates the use of the IAcousticEchoCancellationControl::SetEchoCancellationRenderEndpoint method.

The Acoustic Echo Cancellation APIs provide applications using communications streams the ability to customize
the reference stream of choice to use for echo cancellation. By default, the system uses the default render
device as the reference stream.

The sample demonstrates how an application can check to see if the audio stream provides a way to configure 
echo cancellation. If it does, the interface is used to configure the reference endpoint to be used for echo
cancellation on a communication stream by using the reference stream from a specific render endpoint. 

To use this sample, select a render endpoint from the list (by number),
and the sample use that render endpoint as the reference stream for echo cancellation.

This sample requires Windows 11 build 22540 or later.
    
Sample Language Implementations
===============================
    C++

To build the sample using the command prompt:
=============================================

    1. Open the Command Prompt window and navigate to the directory.
    2. Type msbuild [Solution Filename]
    
To build the sample using Visual Studio (preferred method):
================================================================

    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution.  The application will be built in the default 
       \Debug or \Release directory

To run the sample:
=================
    Type AcousticEchoCancellation.exe at the command line.
