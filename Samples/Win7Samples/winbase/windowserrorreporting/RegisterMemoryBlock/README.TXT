RegisterMemoryBlock Sample
==========================

Demonstrates
------------
This sample demonstrates how to use the WerRegisterMemoryBlock API.

The WerRegisterMemoryBlock API allows an application to include a block of memory as part of the
minidump file that WER generates in the event that an application crashes or hangs.

In this example, we wish for the structure g_StateBlock to be present in the dump file.

Languages
---------
This sample is implemented in the C programming language.

Files
-----
     RegisterMemoryBlock.c              Source code for the sample
     RegisterMemoryBlock.vcproj         Visual C++ 2008 project file
     RegisterMemoryBlock.sln            Visual Studio 2008 solution file

Prerequisites
-------------
Windows SDK v6.0 or newer.
Windows Vista or Windows Server 2008 operating system, or newer.

Building the Sample
-------------------
To build the sample using the command prompt:
     1. Open the Windows SDK command line shell and navigate to the directory.
     2. Type: msbuild RegisterMemoryBlock.sln

To build the sample using Visual Studio 2008 or Visual C++ 2008 Express:
     1. Open Windows Explorer and navigate to the RegisterMemoryBlock directory.
     2. Double-click the icon for the RegisterMemoryBlock.sln file to open the file in Visual Studio.
     3. Add the path to the Windows SDK Include folder to the "Additional Include Directories" setting of the project.
     4. Add the path to the Windows SDK Lib folder to the "Additional Library Directories" setting of the project.
     5. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample
------------------
1. Open a command prompt and navigate to the Release or Debug directory under RegisterMemoryBlock.
2. Run RegisterMemoryBlock.exe (no command-line arguments).

Expected outcome:   The application crashes.
                    Windows Error Reporting UI comes up asking to check for solutions to the problem.
                    (If you are not seeing any UI, then confirm that the DontShowUI and ForceQueue settings are both set
                    to 0. The list of WER settings can be found in the SDK documentation under the "WER Settings" topic).

                    Choosing to check for solutions will collect the file (mylogfile.txt) and add it to the report.

                    The memory block should be present in the dumps inside cab files downloaded from WinQual.

                    As a developer debugging your code, you can make sure that the registered memory block is present in
                    the report's dump file locally, before it is sent out to the WER services backend. You should not try
                    to code the following checks in your code as these are implementation details that are subject to
                    change, and are only meant to help diagnose local issues.
                    To confirm the presence of the registered memory block in the report's dump file locally,
                    1) Force the system to queue the report instead of sending it out. This can be done by:
                        1.1) Removing network connectivity.
                        1.2) OR, setting the ForceQueue setting to 1. Once it is set to 1, there will be no UI when
                             running the sample. This is expected, and the report will be submitted straight to the queue.

                             Be sure to reset the setting to its original value after, as it affects all the applications
                             on your system.

                    2) Open up the Problem Details view of the report that was just submitted by the sample code.
                       The Problem Details view can be reached by double-clicking on the report in the Problem Reports
                       and Solutions control panel applet on Windows Vista/Windows Server 2008, or Action Center on
                       Windows 7/Windows Server 2008 R2 (under the Maintenance, View reliability history, View problem
                       details selection).

                    3) Click the "View a temporary copy of these files" in the view.

                    4) From the opened Windows Explorer window, copy the minidump.mdmp to another folder or open it
                       directly with a debugger of your choice (Windbg/Cdb from the Microsoft Debugging Tools for Windows
                       package, or Visual Studio).

                    5) This step assumes the dump file was opened with Windbg/Cdb. Type the following in the debugger's
                       shell:

                            dt -r1 RegisterMemoryBlock!g_StateBlock

                       You should see both state blocks present in the dump file:

                            0:000> dt -r1 RegisterMemoryBlock!g_StateBlock
                               +0x000 Text             : [256]  "First state block is in the dump."
                               +0x200 SecondStateBlock : 0x00277238 _STATE_BLOCK
                                  +0x000 Text             : [256]  "Second state block is in the dump."
                                  +0x200 SecondStateBlock : (null)


THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
