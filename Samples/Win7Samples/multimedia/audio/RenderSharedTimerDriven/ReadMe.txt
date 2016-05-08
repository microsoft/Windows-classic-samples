========================================================================
    WASAPI Render Shared Timer Driven Sample
========================================================================

This sample demonstrates rendering audio data using WASAPI.

The sample renders audio in shared mode using the timer driven programming model.

Note that this sample requires Windows Vista or later.
    
Sample Language Implementations
===============================
    C++

Files
===============================

WASAPIRenderSharedTimerDriven.vcproj

WASAPIRenderSharedTimerDriven.cpp
    This is the main application source file.  It parses the command line and instantiates a
    CWASAPIRenderer object which actually performs the rendering.
    
WASAPIRenderer.cpp/WASAPIRenderer.h
    Implementation of a class which uses the WASAPI APIs to render a buffer containing audio data.
    
tonegen.h
    Simple function which generates a sine wave.
    
CmdLine.cpp/CmdLine.h
    Command line argument parser.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named WASAPIRenderSharedTimerDriven.pch and a precompiled types file named StdAfx.obj.


To build the sample using the command prompt:
=============================================

    1. Open the Command Prompt window and navigate to the directory.
    2. Type msbuild [Solution Filename]
    
To build the sample using Visual Studio 2008 (preferred method):
================================================================

    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
    3. In the Build menu, select Build Solution.  The application will be built in the default 
       \Debug or \Release directory

To run the sample:
=================
    Type WASAPIRenderSharedTimerDriven.exe at the command line
