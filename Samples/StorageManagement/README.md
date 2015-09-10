Storage management application sample
=====================================

This sample is an end-to-end application that uses the [Windows Storage Management API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh830613) to create a formatted volume on a storage subsystem.

This sample performs the following steps:

1.  Gets the subsystem object from the subsystem name that a user specified.
2.  Creates a storage pool in the storage subsystem using one physical disk.
3.  Creates a virtual disk in the storage pool.
4.  Creates a partition on the virtual disk.
5.  Creates an NTFS volume on the partition.
6.  Formats the volume.

This sample is written in C++ and requires some experience with WMIv2 programming.

It is intended to be used by storage application programmers and assumes familiarity with storage hardware programming and storage industry standards such as the [SMI-S v1.5 specification](%20http://go.microsoft.com/fwlink/p/?linkid=161225), which can be downloaded from the [SNIA website](http://go.microsoft.com/fwlink/p/?linkid=161226).

This sample contains the following files:

-   StorageManagementApplication.cpp
-   StorageManagementApplication.h
-   StorageManagementApplication.sln
-   StorageManagementApplication.vcxproj
-   StorageManagementApplication.vcxproj.filters

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[Windows Storage Management API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh830613)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample, open the solution (.sln) file titled StorageManagementApplication.sln from Visual Studio 2013 for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. Press F7 (or F6 for Visual Studio 2013) or go to **Build-\>Build Solution** from the top menu after the sample has loaded.

Run the sample
--------------

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio for Windows 8.1 (any SKU) or later versions of Visual Studio and Windows. (Or select the corresponding options from the **Debug** menu.)

**Note**  The user needs to specify the name of the subsystem that the system has.

