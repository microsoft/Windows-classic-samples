Setting central access control and resource attributes sample
=============================================================

Demonstrates setting both central access policies and resource attributes to a security descriptor and displaying the access granted by [**AuthzAccessCheck**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375788).

**Note**  This sample was previously entitled as the Resource attributes sample.

The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and Windows Server.

This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**AuthzAccessCheck**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa375788)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample, open the ResourceAttributesSample.sln solution file in Visual Studio 2013 for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. Press F7 (or F6 for Visual Studio 2013) or go to Build-\>Build Solution from the top menu after the sample has loaded.

Run the sample
--------------

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. (Or select the corresponding options from the Debug menu.) When executed without any command line options, the output contains a description of the usage of the tool. To be able to exercise the options involving Central Access Policy IDs or resource claims, you need to run this sample in an environment where claim types and Central Access Policies have been defined for the forest and the Central Access Policy being used is applied to the machine the tool is running on.

