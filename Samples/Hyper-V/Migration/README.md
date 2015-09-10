Hyper-V virtual machine migration sample
========================================

This sample demonstrates how to use the Hyper-V WMI APIs to manage virtual machine migration.The sample demonstrates how to perform each of the following operations:

-   Perform a simple migration of a virtual machine using the [**MigrateVirtualSystemToHost**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859765) method.
-   Perform a simple migration of a virtual machine using destination host IP addresses.
-   Perform a simple migration of a virtual machine that has a new data root.
-   Migrate a planned virtual machine, perform fixup, and migrate the running state to the planned virtual machine.
-   Modify the migration service settings using the [**ModifyServiceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859767) method.
-   Modify the migration service networks using the [**AddNetworkSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859755) method.
-   Check the migration compatibility for a virtual machine, or for two hosts, using the [**GetSystemCompatibilityInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859760) and [**CheckSystemCompatibilityInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859756) methods.
-   Perform a simple check of the migration capability of a virtual machine.
-   Migrate the VHDs and data roots for a virtual machine.
-   Migrate the first VHD for a virtual machine to a new resource pool.
-   Perform a simple migration of a virtual machine, using resource pools to obtain the correct VHD paths.
-   Migrate all virtual machine files to a new location at the destination host.

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

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Migration.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in several different modes.

### Perform a simple migration of a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-simple** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine to migrate.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Perform a simple migration of a virtual machine using destination host IP addresses

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-simple-with-ip** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine to migrate.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Perform a simple migration of a virtual machine with a new data root

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-simple-with-new-root** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine to migrate.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Migrate a planned virtual machine, perform fixup, and migrate the running state to the planned virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-detailed** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine to migrate.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the migration service settings

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe modifyservice** *Host*

    where the parameters are as follows:

    -   *Host* is the name of the host server.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the migration service networks

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe modifynetworks** *Host*

    where the parameters are as follows:

    -   *Host* is the name of the host server.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Check the migration compatibility of two hosts

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe checkcompatibility** *SourceHost* **** *DestinationHost*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host.
    -   *DestinationHost* is the name of the destination host.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Check the migration compatibility of a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe checkcompatibility** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Perform a simple check of the migration capability of a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-simple-check** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Migrate the VHDs and data roots for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe storage-simple** *Host* **** *VmName* **** *NewLocation*

    where the parameters are as follows:

    -   *Host* is the name of the current host of the virtual machine.
    -   *VmName* is the name of the virtual machine.
    -   *NewLocation* is the new location for all VHDs and data roots.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Migrate the first VHD for a virtual machine to a new resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe storage-simple-with-pool** *Host* **** *VmName* **** *NewPoolId* **** *BasePath*

    where the parameters are as follows:

    -   *Host* is the name of the current host of the virtual machine.
    -   *VmName* is the name of the virtual machine.
    -   *NewPoolId* is the identifier of the target resource pool.
    -   *BasePath* is the base directory of the VHD for the resource pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Perform a simple migration of a virtual machine, using resource pools to obtain the correct VHD paths

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-and-storage-simple** *SourceHost* **** *DestinationHost* **** *VmName*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Migrate all virtual machine files to a new location at the destination host

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MigrationSamples.exe vm-and-storage** *SourceHost* **** *DestinationHost* **** *VmName* **** *NewLocation*

    where the parameters are as follows:

    -   *SourceHost* is the name of the current host of the virtual machine.
    -   *DestinationHost* is the name of the destination host.
    -   *VmName* is the name of the virtual machine.
    -   *NewLocation* is the new location for all VHDs and data roots.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

