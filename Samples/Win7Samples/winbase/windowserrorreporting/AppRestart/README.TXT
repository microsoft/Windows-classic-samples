AppRestart Sample
=================

Demonstrates
------------
This sample demonstrates how to use the RegisterApplicationRestart API.

The RegisterApplicationRestart API lets Windows Error Reporting restart the application automatically
in the event that the application crashes or hangs, or has to be restarted due to an update.

In this sample, we register the application for restart. We tell WER to restart the application with the
"/restarted" command-line. This way the application can do custom processing when it is restarted.

A process is eligible for automatic restart only if it had been running for longer than 60 seconds.
This is done to prevent cyclical restarts.

You may also choose to combine automatic application restart with application recovery, as shown in the
AppRecovery sample.

Languages
---------
This sample is implemented in the C programming language.

Files
-----
     AppRestart.c           Source code for the sample
     AppRestart.vcproj      Visual C++ 2008 project file
     AppRestart.sln         Visual Studio 2008 solution file
 
Prerequisites
-------------
Windows SDK v6.0 or newer.
Windows Vista or Windows Server 2008 operating system, or newer.

Building the Sample
-------------------
To build the sample using the command prompt:
     1. Open the Windows SDK command line shell and navigate to the directory.
     2. Type: msbuild AppRestart.sln

To build the sample using Visual Studio 2008 or Visual C++ 2008 Express:
     1. Open Windows Explorer and navigate to the AppRestart directory.
     2. Double-click the icon for the AppRestart.sln file to open the file in Visual Studio.
     3. Add the path to the Windows SDK Include folder to the "Additional Include Directories" setting of the project.
     4. Add the path to the Windows SDK Lib folder to the "Additional Library Directories" setting of the project.
     5. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample
------------------
1. Open a command prompt and navigate to the Release or Debug directory under AppRestart.
2. Run AppRestart.exe (no command-line arguments).

Expected outcome: The application waits 62 seconds before crashing. After crashing, WER restarts the application.


THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
