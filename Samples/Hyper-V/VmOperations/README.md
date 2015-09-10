Virtual Machine operations sample
=================================

This sample shows how to use the Hyper-V WMI APIs to perform various operations on virtual machines.The sample demonstrates how to perform each of the following operations:

-   Inject a non-maskable interrupt into a virtual machine using the [**InjectNonMaskableInterrupt**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280534) method.
-   Configure MMIO gap size for a virtual machine using the [**LowMmioGapSize**](http://msdn.microsoft.com/en-us/library/windows/desktop/) parameter.

This sample is written in C\# and requires some experience with WMI programming.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Hyper-V WMI provider (V2)](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850319)

[Using the Hyper-V WMI provider](http://msdn.microsoft.com/en-us/library/windows/desktop/hh850310)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled vmoperations.sln.

3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

**Note**  This sample must be run as an administrator.

This sample can be run in two different modes:.

### Inject a non-maskable interrupt into a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **VmOperationsSamples.exe InjectNmi** *VmName*

    where the parameters are as follows:

    -   *VmName* is the name of the virtual machine which to send the interrupt. This parameter is required.

2.  To debug the app and then run it from Visual Studio 2013, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

### Configure the MMIO gap size for a virtual machine

To run this sample in this mode, follow these steps.

1.  Enter the debug command line arguments for the project. The usage of this sample is:

    **VmOperationsSamples.exe ConfigureMmioGap***VmName* **** *GapSize*

    where the parameters are as follows:

    -   *VmName* is the name of the virtual machine for which to configure the MMIO gap size. This parameter is required.
    -   *GapSize* is the size of the gap in megabytes. This parameter is required.

2.  To debug the app and then run it from Visual Studio 2013, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

3.  The final result of the operation will be displayed in the console window.

