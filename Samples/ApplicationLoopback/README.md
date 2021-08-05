---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Application loopback audio capture
urlFragment: applicationloopbackaudio-sample
description: Demonstrates how to capture system audio either from a specific process tree or for all process except a process tree.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Application loopback API Capture Sample

This sample demonstrates the use of ActivateAudioInterfaceAsync Win32 API with a new initialization structure. 
The new data structure makes it possible to restrict captured audio data to that rendered by a specific 
process and any of its child processes. Windows 10 has always supported capturing all audio that is played on 
an audio endpoint (referred to as "system" loopback capture), which captures all audio from all apps that 
are playing sounds on the chosen audio endpoint. 

With the new structure, only audio from the specified process, and its children, will be captured. Audio rendered by
other processes will not be captured. A flag is also provided to reverse the behavior, capturing all system
audio *except* those from the the specified process (and its children). Furthermore, the capture is not tied to a 
specific audio endpoint, eliminating the need to create a separate IAudioClient to capture from each physical 
audio endpoint. 

If the processes whose audio will be captured does not have any audio rendering streams, then the capturing 
process receives silence.

To use this sample, obtain the process ID for the process tree you wish to capture or exclude from capture.
You can use Task Manager or the tlist program to get this ID. Run the sample with the process ID, the
desired capture mode (including the process tree or excluding it), and the output WAV file.

Examples:

* Capture audio from process 1234 and its children: `ApplicationLoopback 1234 includetree Captured.wav`
* Capture audio from all process except process 1234 and its children: `ApplicationLoopback 1234 excludetree Captured.wav`

Note that this sample requires Windows 10 build 20348 or later.
    
Sample Language Implementations
===============================
    C++

Files
===============================

ApplicationLoopback.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

ApplicationLoopback.cpp
    This is the main application source file. It parses the command line and instantiates a
    CLoopbackCapture object which actually performs the capturing.
    
LoopbackCapture.cpp/LoopbackCapture.h
    Implementation of a class which uses the WASAPI APIs to capture audio from a process using ActivateAudioInterfaceAsync.
    
Common.h
    Helper for implementing IMFAsyncCallback.


To build the sample using the command prompt:
=============================================

    1. Open the Command Prompt window and navigate to the directory.
    2. Type msbuild [Solution Filename]
    
To build the sample using Visual Studio 2019 (preferred method):
================================================================

    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution.  The application will be built in the default 
       \Debug or \Release directory

To run the sample:
=================
    Type ApplicationLoopback.exe at the command line with appropriate command line options described above.
