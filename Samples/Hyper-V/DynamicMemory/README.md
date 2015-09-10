Hyper-V dynamic memory sample
=============================

This sample demonstrates how to use the Hyper-V WMI APIs to obtain information about, and modify, dynamic memory in a virtual machine.The sample demonstrates how to perform each of the following operations:

-   Obtain dynamic memory usage information for a virtual machine using the [**GetSummaryInformation**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850062) method.
-   Obtain the swap file data root for a virtual machine from the related [**Msvm\_VirtualSystemSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850257) class.
-   Modify the swap file data root for a virtual machine using the [**ModifySystemSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850104) method.

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled DynMem.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in three different modes.

### Obtain dynamic memory usage information for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **DynMem.exe mem-status** *hostMachine* **** *vmName*

    where *hostMachine* is the name of the computer that is hosting the virtual machine and *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Obtain the swap file data root for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **DynMem.exe get-swap-root** *hostMachine* **** *vmName*

    where *hostMachine* is the name of the computer that is hosting the virtual machine and *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the swap file data root for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **DynMem.exe modify-swap-root** *hostMachine* **** *vmName* **** *new-location*

    where *hostMachine* is the name of the computer that is hosting the virtual machine, *vmName* is the name of the virtual machine, and *new-location* is the path to the new swap file data root directory.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

