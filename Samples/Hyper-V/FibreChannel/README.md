Hyper-V virtual Fiber Channel sample
====================================

This sample demonstrates how to use the Hyper-V WMI Fiber Channel APIs to create, configure, and remove storage area networks and virtual Fiber Channel ports.The sample demonstrates how to perform each of the following operations:

-   Create a virtual storage area network (SAN) and assign resources to it using the [**CreatePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203725) method.
-   Delete a virtual SAN using the [**DeletePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203726) method.
-   Modify the settings of a virtual SAN using the [**ModifyPoolSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203728) method.
-   Modify the resources assigned to a virtual SAN using the [**ModifyPoolResources**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203727) method.
-   Create a virtual Fiber Channel port for a virtual machine using the [**Msvm\_VirtualSystemManagementService.AddResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850020) method.
-   Delete a virtual Fiber Channel port from a virtual machine using the [**Msvm\_VirtualSystemManagementService.RemoveResourceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850278) method.
-   Configure the world wide name generator to generate world wide names in a specified range using the [**Msvm\_VirtualSystemManagementService.ModifyServiceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850102) method to modify the **MinimumWWPNAddress**, **MaximumWWPNAddress**, and (optionally) **CurrentWWNNAddress** properties of the [**Msvm\_VirtualSystemManagementServiceSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850254) class.
-   Enumerate the virtual Fiber Channel ports for a virtual machine using the [**Msvm\_FcPortAllocationSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859772) and [**Msvm\_ExternalFcPort**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859768) classes.

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V virtual Fiber Channels API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh859763)

Operating system requirements
-----------------------------

Client

Windows 8

Server

Windows Server 2012

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled FibreChannel.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in several different modes. To obtain a list of the operations, use the following command line:

**FibreChannelSamples.exe /?**

To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

Create a virtual SAN
--------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe CreateSan** *SanName* **(***WWPN* **** *WWNN* **+)** [*"SAN Notes"*]

    where the parameters are as follows:

    -   *SanName* is the name to be applied to the SAN.
    -   *WWPN* is the port name and *WWNN* is the node name for the SAN. You can include one or more pairs of these arguments in sequence.
    -   *SAN Notes* are the notes for the SAN. The notes must be contained inside of quotation marks. This parameter is optional and a standard note will be added if it is not specified.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Delete a virtual SAN
--------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe DeleteSan** *SanName*

    where *SanName* is the name of the SAN to delete.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Modify the settings of a virtual SAN
------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe ModifySanName** *SanName* **** *NewSanName*

    where the parameters are as follows:

    -   *SanName* is the current name of the SAN to be modified.
    -   *NewSanName* is the new name to be applied to the SAN.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Modify the resources assigned to a virtual SAN
----------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe ModifySanPorts** *SanName* **(***WWPN* **** *WWNN* **+)**

    where the parameters are as follows:

    -   *SanName* is the name of the SAN to be modified.
    -   *WWPN* is the new port name and *WWNN* is the new node name to be applied to the SAN. You can include one or more pairs of these arguments in sequence.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Create a virtual Fiber Channel port for a virtual machine
---------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe CreateVirtualFcPort** *VmName* **** *SanName* **** [*WWPN-A*|*WWNN-A*|*WWPN-B*|*WWNN-B*]

    where the parameters are as follows:

    -   *VmName* is the name of the virtual machine to create the virtual Fiber Channel port for.
    -   *SanName* is the name of the SAN.
    -   *WWPN-A* *WWNN-A* *WWPN-B* *WWNN-B* are the port and node names for the virtual Fiber Channel port. These arguments are optional. If they are not included, these names will be auto-generated.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Delete a virtual Fiber Channel port from a virtual machine
----------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe DeleteVirtualFcPort** *VmName* **** *WWPN-A* **** *WWNN-A* **** *WWPN-B* **** *WWNN-B*

    where the parameters are as follows:

    -   *VmName* is the name of the virtual machine to delete the virtual Fiber Channel port from.
    -   *WWPN-A* *WWNN-A* *WWPN-B* *WWNN-B* are the port and node names of the virtual Fiber Channel port to delete.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Configure the world wide name generator
---------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe ConfigureWwnGenerator** *Min WWPN* **** *Max WWPN* **** *New WWNN*

    where the parameters are as follows:

    -   *Min WWPN* is the new minimum world wide port name to use for the **MinimumWWPNAddress** property.
    -   *Max WWPN* is the new maximum world wide port name to use for the **MaximumWWPNAddress** property.
    -   *New WWNN* is the world wide node name to use for the **CurrentWWNNAddress** property.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

Enumerate the virtual Fiber Channel ports for a virtual machine
---------------------------------------------------------------

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **FibreChannelSamples.exe EnumerateFcPorts**

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

