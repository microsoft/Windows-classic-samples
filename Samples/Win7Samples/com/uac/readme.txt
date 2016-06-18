// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

========================================================================
    Win32 APPLICATION : ElevationSample Project overview
========================================================================


Demonstrates
============

This sample demonstrates the usage of the COM Elevation moniker.  It shows the correct usage of the elevation moniker and also provides an example of when using ShellExecute is a better option.  

The moniker is demonstrated through a simple Registration GUI that provides similar functionality to regsvr32.exe and also allows registering self-registration executables.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

Source Files
============

-RegisterServer.idl 				-IDL interface definition for the IRegisterExe and IRegisterDll that will implement registration functionality
-Register.cpp, Register.h			-Implementation of IRegisterExe, IRegisterDll, and RegistrationClass, demonstrates elevation through ShellExecute in function VerifyAndExecuteExe
						-Demonstrates registry keys required for elevation including:
							{CLSID}\LocalizedString,	 (required)
							{CLSID}\Elevation\Enabled,	 (required)
							{CLSID}\Elevation\IconReference, (optional)
							{AppId}\AccessPermission	 (optional, required for Over The Shoulder Elevation)  	
-WinMain.cpp, WinMain.h		-Implements the user interface and the main function for the sample.
-ElevationManager.cpp, ElevationManager.h	-Implements functionality for elevation including using the elevation moniker and setting a button shield icon.

Other Files
===========
-mui.rcconfig					-Required for the localized string resource needed for elevation.  See the resources configuration command line and post build events in the project properties for more information about c

 
Prerequisites
=============
Visual Studio must be configured to use the Windows Vista (or later) SDK to compile.
Application must run on Windows Vista or later.


Building the Sample
===================


To build the sample using the command prompt:
=============================================
	1. Open the command prompt from Start → All Programs → Microsoft Visual Studio 2005 → Visual Studio Tools → Visual Studio 2005 Command Prompt. 
	2. Navigate to the folder containing the application sample source code, and type the following: 

	vcbuild MUIAppSample.sln

	3. The above procedure builds the sample for all available configurations and platforms. For more information about controlling the command line build, refer to the Vcbuild.exe command line help by typing the following:

	vcbuild /?
	
	4. From an elevated command prompt browse to the project output directory (ElevationSample\Debug, or ElevationSample\Release).  
		On 32-bit windows: Type regsvr32 RegisterServer.dll
		On 64-bit windows: For 32-bit DLL use syswow64\regsvr32.exe, For 64-bit DLL use system32\regsvr32.exe.



To build the sample using Visual Studio 2005 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
     4. From an elevated command prompt browse to the project output directory (ElevationSample\Debug, or ElevationSample\Release).  
		On 32-bit windows: Type regsvr32 RegisterServer.dll
		On 64-bit windows: For 32-bit DLL use syswow64\regsvr32.exe, For 64-bit DLL use system32\regsvr32.exe.


Running the Sample
==================
1) In Visual Studio click Debug->Start without debugging.  Or browse to the output directory and run ElevationSample.exe

note:  For the correct elevation behavior do not run from visual studio while running as administrator.  For the correct elevation behavior do not use the "Run as Administrator" option when executing the file manually.  While running as administrator the application will work correctly, but it will not show you the elevation dialog.

2) Enter the relative path of a COM Server or use the Browse dialog to select a server to use.

3) Select register or unregister.

4) Click Ok.

5) Click allow when prompted for elevation.  For over the shoulder elevation, enter the administrator user credentials.

6) If a dialog alerts you of success your COM server is now registered/unregistered.
