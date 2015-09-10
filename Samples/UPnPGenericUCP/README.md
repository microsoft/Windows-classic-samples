UPnP control point sample
=========================

This sample shows device discovery and subscription modes for Universal Plug and Play (UPnP), along with querying variables, events, and invoking actions.

This sample uses the [UPnP Control Point API](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381109) provided by **upnp.dll**. The sample can discover devices on the network by using one of the three types of searches available: **FindByType**, **FindByUDN** and **AsyncFind**.

The devices found by these search capabilities are instantiated in the device list. When a device is selected, service objects for the device are enumerated and listed in the service list. If the **Delay Subscription** check box is selected when the desired device is selected, the **Subscribe** button will become active and SCPD download and event subscription will be delayed when the services are enumerated. If the **Delay Subscription** check box is not selected, the SCPD will be downloaded and the subscriptions will be done while enumerating services.

One of the services can be selected and controlled by invoking actions against it. The events relevant to the service are displayed in the events field. If the **Delay Subscription** check box was selected when the device was selected, click the **Subscribe** button to start event subscriptions. If **Asynchronous Control** check box is selected, the asynchronous control methods will be used. If the **Asynchronous Control** check box is not selected, the normal synchronous methods will be used.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[UPnP Control Point API](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381109)

Related technologies
--------------------

[UPnP Control Point API](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381109)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  2. Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2.  Copy the **devType.txt** and **Udn.txt** files from the CPP directory to the current directory.
3.  Run **GenericUcp.exe**. The device types and the UDNs that appear in the drop down menu are from **devType.txt** and **Udn.txt** respectively.

