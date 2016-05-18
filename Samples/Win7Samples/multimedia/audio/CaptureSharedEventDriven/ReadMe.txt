========================================================================
    WASAPI Capture Shared Event Driven Sample
========================================================================

This sample demonstrates capturing audio data using WASAPI.

The sample captures audio in shared mode using the event driven programming model, saving the captured data to a file.

Note that this sample requires Windows Vista or later.
    
Sample Language Implementations
===============================
    C++

Files
===============================

WASAPICaptureSharedEventDriven.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

WASAPICaptureSharedEventDriven.cpp
    This is the main application source file.  It parses the command line and instantiates a
    CWASAPICapture object which actually performs the rendering.
    
WASAPICapture.cpp/WASAPICapture.h
    Implementation of a class which uses the WASAPI APIs to render a buffer containing audio data.
    
CmdLine.cpp/CmdLine.h
    Command line argument parser.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named WASAPICaptureSharedEventDriven.pch and a precompiled types file named StdAfx.obj.
targetver.h
    Specifies the OS target version for the sample (Vista+)


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
    Type WASAPICaptureSharedEventDriven.exe at the command line
