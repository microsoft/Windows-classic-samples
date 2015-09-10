Portable Devices Services COM API sample
========================================

This sample shows how to use the Windows Portable Device Services COM API (IPortableDeviceService).

This app demonstrates: the enumeration of portable device services, the enumeration of content for a given service, the querying of capabilities for a given service, setting or retrieving properties for a given service, and invoking methods on a given service.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and doesn't compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

Open Windows Explorer and navigate to the directory containing the sample. Double-click the icon for the .sln (solution) file to open the file in Visual Studio. In the Build menu, select Build Solution. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

Navigate to the directory that contains the new executable, using the command prompt. Type "WpdServiceApiSample.exe" at the command line.

This sample works bests with a Portable Device that supports the Contacts Device Service connected to the system, such as the Media Transfer Protocol (MTP) Device Simulator.

