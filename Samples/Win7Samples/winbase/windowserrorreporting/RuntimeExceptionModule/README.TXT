RuntimeExceptionModule Sample
=============================

Demonstrates
------------
This sample demonstrates how to use the runtime exception module technology introduced by WER in Windows 7
and Windows Server 2008 R2.

The runtime exception module is an out-of-process DLL that WER loads when a crash or an unhandled exception occurs.
The DLL implements and exports the following functions:

    OutOfProcessExceptionEventCallback
    OutOfProcessExceptionEventSignatureCallback
    OutOfProcessExceptionEventDebuggerLaunchCallback

    (The DLL must also include the DllMain entry point.)

A handler DLL must be of the same platform as the exception it is handling: that is, for 32-bit x86 applications,
a 32-bit x86 DLL must be provided. For 64-bit x64 applications, a 64-bit x64 DLL must be provided.

Languages
---------
This sample is implemented in the C programming language.

Files
-----
     CrashExe\CrashExe.c            Source code for the crashing EXE whose crash is handled by HandlerDll
     CrashExe\CrashExe.vcproj       Visual C++ 2008 project file for CrashExe

     HandlerDll\HandlerDll.c        Source code for the runtime exception module which handles special crashes
     HandlerDll\HandlerDll.def      Definition file declaring which functions are exported by HandlerDll.dll
     HandlerDll\HandlerDll.vcproj   Visual C++ 2008 project file for HandlerDll

     RuntimeExceptionModule.sln     Visual Studio 2008 solution file containing both projects

Prerequisites
-------------
Windows SDK v7.0 or newer.
Windows 7 or Windows Server 2008 R2 operating system, or newer.

Building the Sample
-------------------
To build the sample using the command prompt:
     1. Open the Windows SDK command line shell and navigate to the directory.
     2. Type: msbuild RuntimeExceptionModule.sln

To build the sample using Visual Studio 2008 or Visual C++ 2008 Express:
     1. Open Windows Explorer and navigate to the RuntimeExceptionModule directory.
     2. Double-click the icon for the RuntimeExceptionModule.sln file to open the file in Visual Studio.
     3. Add the path to the Windows SDK Include folder to the "Additional Include Directories" setting of the project.
     4. Add the path to the Windows SDK Lib folder to the "Additional Library Directories" setting of the project.
     5. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Preparing the System
--------------------
Prior to using the runtime exception module, the module must be registered with the OS by modifying the system registry:

1. Create a new key named RuntimeExceptionHelperModules under:
    a. HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting
    b. For x86 applications on a 64-bit machine:
       HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\Windows Error Reporting\

2. Create a new DWORD value and name it after the full path of your handler DLL.

For example, if your runtime exception module is placed under C:\MyHandler\HandlerDll.dll, then create a DWORD
value named "C:\MyHandler\HandlerDll.dll". The actual contents of the DWORD value are ignored.

Running the Sample
------------------
1. Open a command prompt and navigate to the Release or Debug directory under RuntimeExceptionModule.
2. Run CrashExe.exe (no command-line arguments).

Expected outcome:   The application crashes. When observing the problem signature under the 'View problem details'
                    view of the Windows Error Reporting UI, the following sample signatures are present:

                        Problem signature:
                          Problem Event Name:   MySampleEventName
                          one:  111
                          two:  222



THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved.
