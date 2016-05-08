========================================================================
    WASAPI Endpoint Volume Sample
========================================================================

This sample demonstrates changing the default endpoint volume using the WASAPI EndpointVolume APIs.

The sample changes the volume up, down or sets the volume to a specific value.

Note that this sample requires Windows Vista or later.
    
Sample Language Implementations
===============================
    C++

Files
===============================
    EndpointVolumeChanger.cpp
        This is the main application source file.  It parses the command line and activates an
        EndpointVolume object which actually performs the volume change based on command line parameters.
        
    CmdLine.cpp/CmdLine.h
        Command line argument parser.

    targetver.h
    stdafx.h
    stdafx.cpp
    EndpointVolumeChanger.vcproj
    EndpointVolumeChanger.sln

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the EndpointVolume directory.
     2. Type msbuild EndpointVolumeChanger.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the EndpointVolume directory.
     2. Double-click the icon for the EndpointVolumeChanger.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     Type EndpointVolumeChanger.exe at the command line, or double-click the icon for EndpointVolumeChanger.exe to launch it from Windows Explorer.
