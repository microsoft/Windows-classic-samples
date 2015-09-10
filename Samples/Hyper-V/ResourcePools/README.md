Hyper-V resource pool management sample
=======================================

This sample demonstrates how to use the Hyper-V WMI APIs to manage Hyper-V resource pools.The sample demonstrates how to perform each of the following operations:

-   Enumerate and display the resources supported by this sample.
-   Create a resource pool using the [**CreatePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203725) method.
-   Display resources for a resource pool using the [**Msvm\_ResourceAllocationSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850200) class.
-   Modify the host resources in a resource pool using the [**ModifyPoolResources**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203727) method.
-   Display the setting data for a resource pool using the [**Msvm\_ResourcePoolSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203732) class.
-   Modify the setting data for a resource pool using the [**ModifyPoolSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203728) method.
-   Delete a resource pool using the [**DeletePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203726) method.
-   Display the [**Msvm\_ResourcePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850201), [**Msvm\_ResourcePoolSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/jj203732) and [**Msvm\_ResourceAllocationSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850200) properties for a resource pool.
-   Display the pool identifiers of the child pools for a resource pool.
-   Display the pool identifiers of the parent pools for a resource pool.
-   Display the allocation capabilities for a resource pool.

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

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled ResourcePools.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in 12 different modes.

### Enumerate and display the resources supported by this sample

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe EnumerateSupportedResources**

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Create a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe CreatePool** *ResourceName* **** *PoolId* **** *PoolName* **** *NewParentPoolIds* **** *ParentPoolHostResources*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.
    -   *PoolName* is the name of the new pool.
    -   *NewParentPoolIds* is a delimited string containing the parent pool identifiers. Each identifier is contained in the "[p]" delimiter. For example, "[p]Pool A[p][p]Pool B[p]".
    -   *ParentPoolHostResources* is a delimited string containing the host resources for each parent pool. This is a string representation of a two-dimensional array, where each host resource is contained in an "[h]" delimiter and each pool is contained in a "[p]" delimiter. For example, "[p][h]Child A, Resource 1[h][h]Child A, Resource 2[h][p][p][h]Child B, Resource 1[h][p]".

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display resources for a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayPoolResources** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the host resources in a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe ModifyPoolResources** *ResourceName* **** *PoolId* **** *NewParentPoolIds* **** *ParentPoolHostResources*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.
    -   *PoolName* is the name of the new pool.
    -   *NewParentPoolIds* is a delimited string containing the parent pool identifiers. Each identifier is contained in the "[p]" delimiter. For example, "[p]Pool A[p][p]Pool B[p]".
    -   *ParentPoolHostResources* is a delimited string containing the host resources for each parent pool. This is a string representation of a two-dimensional array, where each host resource is contained in an "[h]" delimiter and each pool is contained in a "[p]" delimiter. For example, "[p][h]Child A, Resource 1[h][h]Child A, Resource 2[h][p][p][h]Child B, Resource 1[h][p]".

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display the setting data for a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayPoolSettings** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Modify the setting data for a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe ModifyPoolSettings** *ResourceName* **** *PoolId* **** *NewPoolId* **** *NewPoolName*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.
    -   *NewPoolId* is the new pool identifier.
    -   *NewPoolName* is the new pool name.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Delete a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DeletePool** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display verbose information for a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayPool** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display child pools

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayChildPools** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display parent pools

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayParentPools** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Display the allocation capabilities for a resource pool.

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **ResourcePoolSamples.exe DisplayAllocationCapabilities** *ResourceName* **** *PoolId*

    where the parameters are as follows:

    -   *ResourceName* is the friendly name of the resource type. The friendly names of the resources can be obtained by running this sample with the "EnumerateSupportedResources" option.
    -   *PoolId* is the pool identifier for the new pool.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

