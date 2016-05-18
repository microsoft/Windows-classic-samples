Add Dump Sample
===============

Demonstrates
------------
This sample demonstrates how to use the Windows Error Reporting APIs to send
a report that contains a dump file of the current process.

Languages
---------
This sample is implemented in the C programming language.

Files
-----
     AddDumpSample.c            Source code for the sample
     AddDumpSample.vcproj       Visual C++ 2008 project file
     AddDumpSample.sln          Visual Studio 2008 solution file
 
Prerequisites
-------------
Windows SDK v6.0 or newer.
Windows Vista or Windows Server 2008 operating system, or newer.

Building the Sample
-------------------
To build the sample using the command prompt:
     1. Open the Windows SDK command line shell and navigate to the directory.
     2. Type: msbuild AddDumpSample.sln

To build the sample using Visual Studio 2008 or Visual C++ 2008 Express:
     1. Open Windows Explorer and navigate to the AddDumpSample directory.
     2. Double-click the icon for the AddDumpSample.sln file to open the file in Visual Studio.
     3. Add the path to the Windows SDK Include folder to the "Additional Include Directories" setting of the project.
     4. Add the path to the Windows SDK Lib folder to the "Additional Library Directories" setting of the project.
     5. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample
------------------
1. Open a command prompt and navigate to the Release or Debug directory under AddDumpSample.
2. Run AddDumpSample.exe (no command-line arguments).

Expected outcome:   Windows Error Reporting UI comes up asking to check for solutions to the problem.
                    (If you are not seeing any UI, then confirm that the DontShowUI and ForceQueue settings are both set
                    to 0. The list of WER settings can be found in the SDK documentation under the "WER Settings" topic).

                    Choosing to check for solutions will collect the process dump and add it to the report.

                    The dump file should be present in cab files downloaded from WinQual.

                    There are many conditions which can cause WerReportAddDump to succeed, but for no dump to be
                    collected when submission actually happens. As a developer debugging your code, you can make sure
                    that the process dump is present in the report locally, before it is sent out to the WER services
                    backend. You should not try to code the following checks in your code as these are implementation
                    details that are subject to change, and are only meant to help diagnose local issues.
                    To confirm the presence of the process dump in the report locally,
                    1) Force the system to queue the report instead of sending it out. This can be done by:
                        1.1) Removing network connectivity.
                        1.2) OR, setting the ForceQueue setting to 1. Once it is set to 1, there will be no UI when
                             running the sample. This is expected, and the report will be submitted straight to the queue.
                             
                             Be sure to reset the setting to its original value after, as it affects all the applications
                             on your system.

                    2) Check the Problem Details of the report that was just submitted by the sample code.
                       The Problem Details view can be reached by double-clicking on the report in the Problem Reports
                       and Solutions control panel applet on Windows Vista/Windows Server 2008, or Action Center on
                       Windows 7/Windows Server 2008 R2 (under the Maintenance, View reliability history, View problem
                       details selection). The problem details should list the process dump as one of the files in the
                       report, like so:

                            Description
                                Sample Problem Description

                            Problem signature
                                Problem Event Name:	SampleGenericReport
                                Param1:	Value1
                                Param2:	Value2
                                Param3:	Value3
                                OS Version:	6.1.7105.2.0.0.256.1
                                Locale ID:	1033

                            Files that help describe the problem
                                minidump.mdmp


THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
