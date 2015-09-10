Hyper-V metrics sample
======================

This sample demonstrates how to use the Hyper-V metrics WMI APIs to manage metrics for a virtual machine.The sample demonstrates how to perform each of the following operations:

-   Enable metrics gathering for a virtual machine using the [**ControlMetrics**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850034) method.
-   Disable metrics gathering for a network adapter using the [**ControlMetrics**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850034) method.
-   Set the metrics flush interval for all virtual machines using the [**ModifyServiceSettings**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850100) method and the [**Msvm\_MetricServiceSettingData**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850183) class.
-   Determine if metrics collection is enabled for a virtual machine using the [**Msvm\_MetricDefForME**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850178) class.
-   Enumerate the discrete metrics for a virtual machine using the related [**Msvm\_AggregationMetricValue**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850109) classes.
-   Enumerate the metrics for a resource pool using the related [**Msvm\_AggregationMetricDefinition**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850108) and [**Msvm\_AggregationMetricValue**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850109) classes.

This sample is written in C\# and requires some experience with WMI programming.

The Windows-classic-samples repo includes a variety of code samples that demonstrate the use of various new programming features for managing Hyper-V that are available starting in Windows 8.1 and/or Windows Server 2012 R2. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more information about the programming models, platforms, languages, and APIs demonstrated in this sample, please refer to the [Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319) documentation.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

Related technologies
--------------------

[Hyper-V metrics API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850072)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled Metrics.sln.

3.  Press F7 (or F6 for Microsoft Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in six different modes.

### Enable metrics gathering for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe EnableMetricsForVm** *vmName*

    where *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Disable metrics gathering for a network adapter

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe DisableMetricsForNetworkAdapter** *MacAddress* **** *IpAddress*

    where the parameters are as follows:

    -   *MacAddress* is the MAC address of the network adapter.
    -   *IpAddress* is the IP address of the port ACL to disable metrics for. This network port ACL must have been previously added to the virtual machine.

2.  To debug the app and then run it from Visual Studio., press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Set the metrics flush interval for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe ConfigureMetricsFlushInterval** *flush-interval-hours*

    where *flush-interval-hours* is the new metrics flush interval, in hours.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Determine if metrics collection is enabled for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe QueryMetricCollectionEnabledForVm** *vmName*

    where *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Enumerate the discrete metrics for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe EnumerateDiscreteMetricsForVm** *vmName*

    where *vmName* is the name of the virtual machine.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Enumerate the metrics for a resource pool

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **MetricsSample.exe EnumerateMetricsForResourcePool** *ResourceType* **** *ResourceSubType* **[***PoolId***]**

    where *ResourceType* is the numeric resource type, *ResourceSubType* is the resource sub type, and *PoolId* is the resource pool identifier. The *PoolId* is optional.

2.  To debug the app and then run it from Visual Studio, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

