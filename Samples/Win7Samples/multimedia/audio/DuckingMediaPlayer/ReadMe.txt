========================================================================
    WIN32 APPLICATION : Ducking Media Player Sample Project Overview
========================================================================

This sample implements a simple media player that responds to the "ducking" 
feature in Windows 7.  It also implements a volume control which tracks
to the volume control in the volume mixer. 

Note that this sample requires Windows 7 or later.

Sample Language Implementations
===============================
    C++
    
Files
===============================
    MediaPlayer.h
    resource.h
    stdafx.h
    targetver.h
    DuckingMediaPlayerSample.cpp
        This is the main application source file.
    MediaPlayer.h, MediaPlayer.cpp
        Implements the simple media player.

    stdafx.cpp
    DuckingMediaPlayerSample.rc
    DuckingMediaPlayerSample.vcproj
    DuckingMediaPlayerSample.sln

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the DuckingMediaPlayer directory.
     2. Type msbuild DuckingMediaPlayerSample.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the DuckingMediaPlayer directory.
     2. Double-click the icon for the DuckingMediaPlayerSample.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type DuckingMediaPlayerSample.exe at the command line, or double-click the icon for DuckingMediaPlayerSample.exe to launch it from Windows Explorer.
