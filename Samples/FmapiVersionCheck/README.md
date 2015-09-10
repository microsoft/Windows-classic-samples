FMAPI version check sample
==========================

This sample shows how to use the FMAPI [**CreateFileRestoreContext**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd239109) and [**CloseFileRestoreContext**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd239108) functions to check for a specific FMAPI version.

This sample is written in C++.

**Note**  FMAPI can only be used in the Windows Preinstallation Environment (WinPE). Applications that use FMAPI must license WinPE.

This sample contains the following files:

-   FmapiVersionCheck.cpp
-   FmapiVersionCheck.sln
-   FmapiVersionCheck.vcxproj

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[File Management API (FMAPI)](http://msdn.microsoft.com/en-us/library/windows/desktop/dd239113)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample from the command prompt:

1.  Open the Command Prompt window and navigate to the FmapiVersionCheck directory.
2.  Type the following command at the command prompt: **msbuild FmapiVersionCheck.sln**.

To build this sample using Microsoft Visual Studio:

1.  Open File Explorer and navigate to the FmapiVersionCheck directory.
2.  Double-click the icon for FmapiVersionCheck.sln to open the file in Visual Studio.
3.  In the **Build** menu, click **Build solution**. The sample will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

**Note**  All FMAPI samples must be run in a Windows 8.1 or Windows Server 2012 R2 WinPE environment.

To run the sample:

1.  Navigate to the directory that contains the new executable file, using the command prompt or File Explorer.
2.  Copy the executable file into a directory that is accessible from WinPE.
3.  Boot to WinPE and open a Command Prompt window. Navigate to where the executable file is located.
4.  Type the name of the executable file at the command prompt.

