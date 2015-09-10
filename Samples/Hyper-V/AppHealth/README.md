Hyper-V application health monitoring sample
============================================

This sample demonstrates how to monitor the health of guest applications running in a virtual machine of a Hyper-V server.Specifically, this sample is a command line application that demonstrates the use of the [**Msvm\_HeartbeatComponent**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850157) class in the [Hyper-V application health monitoring API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850067).

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V application health monitoring API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850067)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled AppHealth.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

To run this sample, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **AppHealth.exe status** *hostMachine* **** *vmName*

    where *hostMachine* is the name of the computer that is hosting the virtual machine and *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The application health status for the specified virtual machine will be displayed in the console window.

