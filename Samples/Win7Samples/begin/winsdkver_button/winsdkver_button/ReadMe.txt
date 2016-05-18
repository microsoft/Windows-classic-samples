Sample Name
WinSDKVersionSample

Demonstrates
This sample provides a way to exercise the new WinSDKVer.h in the Windows SDK. The implementation of the split button in the sample will change based on the version of Windows specified by MAXWINVER in code.

See WinSDKVer.h for more examples of code that can be toggled.

Languages
For example: 

Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

Files
=====
WinSDKVer_Button.vcproj
    This is the main project file for this sample.

WinSDKVer_Button.cpp
    This is the main application source file that contains the entry point
    for this application.

targetver.h
    Defines the minimum required platforms.  This file now uses the values found
    in WinSDKVer.h in order to dynamically set the platform values to the versions
    supported by your current SDK.

Readme.txt
    This file.

/////////////////////////////////////////////////////////////////////////////
AppWizard has created the following resources:

WinSDKVer_Button.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

WinSDKVer_Button.ico
    This is an icon file, which is used as the application's icon (32x32).
    This icon is included by the main resource file WinSDKVer_Button.rc.

small.ico
    This is an icon file, which contains a smaller version (16x16)
    of the application's icon. This icon is included by the main resource
    file WinSDKVer_Button.rc.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named WinSDKVer_Button.pch and a precompiled types file named StdAfx.obj.

Prerequisites
None

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild [Solution Filename].


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


