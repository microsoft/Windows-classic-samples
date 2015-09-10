Hyper-V planned virtual machines sample
=======================================

This sample demonstrates how to use the Hyper-V WMI APIs to manage planned virtual machines.The sample demonstrates how to perform each of the following operations:

-   Create a planned virtual machine using the [**ImportSystemDefinition**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850082) method.
-   Import snapshots for a planned virtual machine using the [**ImportSnapshotDefinitions**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850081) method.
-   Validate a planned virtual machine using the [**ValidatePlannedSystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850311) method.
-   Realize a planned virtual machine using the [**RealizePlannedSystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850268) method.
-   Remove a planned virtual machine using the [**DestroySystem**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850045) method.
-   Fix the VHD and snapshot paths for a planned virtual machine using the [**ModifyResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850099) method.

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

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled PVM.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in six different modes.

### Create a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe ImportVm** *VmDefinitionPath* **[***SnapshotFolderPath***] [***NewId\<True/False\>***]**

    where the parameters are as follows:

    -   *VmDefinitionPath* is the path to the system definition file (.xml or .exp) representing the virtual machine which is to be imported. This parameter is required.
    -   *SnapshotFolderPath* is the path to the folder where the snapshot configurations for this virtual machine can be found. This parameter is optional and, if it is not specified, *VmDefinitionPath* will be used.
    -   *NewId\<True/False\>* indicates whether to reuse the unique identifier for the virtual machine. Pass "True" to create a new identifier or "False" to reuse the existing identifier. This parameter is optional and, if it is not specified, a new identifier will be created.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Import snapshots for a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe ImportSnapshots** *PvmName* **** *SnapshotFolderPath*

    where the parameters are as follows:

    -   *PvmName* is the name of the planned virtual machine to import snapshots for. This parameter is required.
    -   *SnapshotFolderPath* is the path to the folder where the snapshot configurations for this virtual machine can be found. This parameter is required.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Validate a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe ValidatePvm** *PvmName*

    where *PvmName* is the name of the planned virtual machine to validate.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Realize a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe RealizePvm** *PvmName*

    where *PvmName* is the name of the planned virtual machine to be realized.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Remove a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe RemovePvm** *PvmName*

    where *PvmName* is the name of the planned virtual machine to be removed

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Fix the VHD and snapshot paths for a planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **PVMSamples.exe FixVHDPaths** *PvmName* **** *VhdFolderPath*

    where the parameters are as follows:

    -   *PvmName* is the name of the planned virtual machine to repair the VHD and snapshot paths for. This parameter is required.
    -   *VhdFolderPath* is the path to the folder where the VHDs and snapshot snapshots for this virtual machine can be found. This parameter is required.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

