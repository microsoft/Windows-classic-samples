Web service discovery (WS-Discovery) sample
===========================================

This sample shows how to use the Web Service Discovery API to perform WS-Discovery routines by using the [**IWSDiscoveryProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa386012), [**IWSDiscoveryProviderNotify**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa386013), [**IWSDiscoveryPublisher**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa386025) and [**IWSDiscoveryPublisherNotify**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa386026) interfaces.

This sample implements the following WS-Discovery message pattern:

**Probe --\>ProbeMatches --\> Resolve --\> ResolveMatches**.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

Web Services for Devices API

Related technologies
--------------------

Web Services for Devices API

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using Visual Studio (preferred method):

1.  Open **Windows Explorer** and navigate to the sample **\\cpp** directory.
2.  Double-click the icon for the **WSDiscovery.sln** (solution) file to open the file in Visual Studio.
3.  On the **Build** menu, click **Build Solution**. The application will be built in the default **\\Debug** or **\\Release** directory.

To build the sample using the command prompt:

1.  Open the **Command Prompt** window and navigate to the directory containing the sample for a specific language.
2.  Type **msbuild WSDiscovery.sln**.

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable, using the **command prompt** or **Windows Explorer**.
2.  To start the WS-Discovery Client, type **WSDiscoveryClient.exe /?** for a list of commands and usages.
3.  To start the WS-Discovery Target Service, type **WSDiscoveryService.exe /?** for a list of commands and usages.

**Note**  

In order for the client and target service to run correctly, make sure that your firewall settings allow WS-Discovery traffic and that the firewall allows multicast traffic.

The client and target service can be run on the same or different machines, and they can be run it multiple instances simultaneously to mimic the effect of having multiple target services or clients in the network, provided that the machines hosting these applications have sufficient resources such as large enough memory.

