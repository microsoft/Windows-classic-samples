UPnP device registration sample
===============================

This sample implements the **UPnP Dimmer device sample** functionality as a COM server object and demonstrates how to register and unregister the device with the Microsoft UPnP framework device host.

**Note**  The actual device functionality is implemented in the [UPnP Dimmer device sample](http://go.microsoft.com/fwlink/p/?linkid=245629). The UPnP device registration sample will not build if the Dimmer device sample has not been built first.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and doesn't compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Universal Plug and Play API](http://msdn.microsoft.com/en-us/library/windows/desktop/aa382303)

Related technologies
--------------------

[Universal Plug and Play API](http://msdn.microsoft.com/en-us/library/windows/desktop/aa382303)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using Visual Studio (preferred method):

1.  Build the [UPnP Dimmer device sample](http://go.microsoft.com/fwlink/p/?linkid=245629) project and place it in the UPnP device registration sample folder.
2.  Navigate to the UPnP device registration sample **\\cpp** directory.
3.  Double-click the icon for the **RegDevice.sln** (solution) file to open the file in Visual Studio.
4.  In the **Build** menu, click **Build Solution**. The application will be built in the default **\\Debug** or **\\Release** directory.
    **Note**  The built [UPnP Dimmer device sample](http://go.microsoft.com/fwlink/p/?linkid=245629) must be present in the UPnP device registration sample folder at build.

To build the sample using the command prompt:

1.  Open the **Command Prompt** window and navigate to the sample directory.
2.  Type **msbuild RegDevice.sln**.
    **Note**  The built [UPnP Dimmer device sample](http://go.microsoft.com/fwlink/p/?linkid=245629) must be present in the UPnP device registration sample folder at build.

Run the sample
--------------

Running the Sample:

1.  Open a **Command Prompt** window and navigate to the **Release** or **Debug** directory for the built Dimmer sample.
2.  Run **regsvr32 UPNPSampleDimmerDevice.dll** to register the device specific COM object.
3.  Navigate to the **Release** or **Debug** directory for the built Register Device sample.
4.  Copy the **DimmerDevice-Desc.xml** and **DimmingService\_SCPD**.xml files from the RegisterDevice directory to the current directory.
5.  Run **RegDevice.exe**.
6.  After the device is hosted, go to **My Computer** on a computer that has Windows 8.1 installed.
7.  Click **Network** in the left-side menu.
8.  Turn on Network Discovery, if prompted.
9.  The **UPNP SDK Dimmer Device Hosted by Windows** should appear under **Other Devices**.
10. Double-click the device in **Network Explorer**. This should open a presentation web-page in a web browser.
11. Accept the security warning in order to be able to control the device. You can now use the presentation web-page interface to control the device.

