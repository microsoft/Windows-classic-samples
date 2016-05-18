========================================================================
    WIN32 APPLICATION : Ducking Capture Sample Project Overview
========================================================================

This sample implements a simple "Chat" that demonstrates to the "ducking" 
feature in Windows 7.  It simply captures samples from the sound card and 
discards them.

Note that this sample requires Windows 7 or later.

Sample Language Implementations
===============================
    C++

Files
===============================
    ChatTransport.h
    resource.h
    stdafx.h
    targetver.h
    WasapiChat.h
    WaveChat.h
    DuckingCaptureSample.cpp
    stdafx.cpp
    WasapiChat.Cpp
    WaveChat.cpp
    DuckingCaptureSample.rc
    DuckingCaptureSample.vcproj
    DuckingCaptureSample.sln

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the DuckingCaptureSample directory.
     2. Type msbuild DuckingCaptureSample.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the DuckingCaptureSample directory.
     2. Double-click the icon for the DuckingCaptureSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type DuckingCaptureSample.exe at the command line, or double-click the icon for DuckingCaptureSample.exe to launch it from Windows Explorer.
