DXGI desktop duplication sample
===============================

This sample demonstrates how to use the desktop duplication behaviors exposed by DXGI in DirectX.

This sample is written in C++. You also need some experience with DirectX.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

For info about how to use the desktop duplication API, see [Desktop Duplication API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh404487).

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample, open the solution (.sln) file titled DesktopDuplication.sln from Visual Studio 2013 for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. Press F7 (or F6 for Visual Studio 2013) or go to Build-\>Build Solution from the top menu after the sample has loaded.

Run the sample
--------------

To run this sample after building it, perform the following:

1.  Navigate to the directory that contains the new executable, using the command prompt.
2.  Type one of the following commands to run the executable.
    1.  From the command-line, run **desktopduplication.exe parameter \\bitmap [interval in seconds]** to produce a bitmap every [interval in seconds] seconds
    2.  From the command-line, run **desktopduplication.exe \\output [all, \#]** where "all" duplicates the desktop to all outputs, and [\#] specifies the number of outputs.


