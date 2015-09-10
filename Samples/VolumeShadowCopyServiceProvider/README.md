Volume Shadow Copy Service hardware provider sample
===================================================

This sample demonstrates the use of the [Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832) (VSS) COM interfaces to create a VSS hardware provider.

This sample is written in C++ and requires some experience with COM.

This sample contains the following files:

-   async.h
-   eventlogmsgs.mc
-   install-sampleprovider.cmd
-   readme.txt
-   resource.h
-   sampleprovider.cpp
-   sampleprovider.h
-   sampleprovider.rgs
-   setup.txt
-   stdafx.cpp
-   stdafx.h
-   uninstall-sampleprovider.cmd
-   utility.cpp
-   utility.h
-   vsssampleprovider.cpp
-   vsssampleprovider.def
-   vsssampleprovider.idl
-   vsssampleprovider.rc
-   vsssampleprovider.rgs
-   VssSampleProvider.sln
-   VssSampleProvider.vcxproj
-   VssSampleProvider.vcxproj.filters
-   vstorinterface.h

This sample also requires the following files in the Windows SDK:

-   register\_app.vbs
-   virtualstoragevss.sys
-   vssampleprovider.dll
-   vstorcontrol.exe
-   vstorinterface.dll

The following VSS API elements are used or implemented in the sample:

-   [**IVssAdmin::RegisterProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381923)
-   [**IVssHardwareSnapshotProvider::AreLunsSupported**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384241)
-   [**IVssHardwareSnapshotProvider::BeginPrepareSnapshot**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384243)
-   [**IVssHardwareSnapshotProvider::FillInLunInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384245)
-   [**IVssHardwareSnapshotProvider::GetTargetLuns**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384251)
-   [**IVssHardwareSnapshotProvider::LocateLuns**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384256)
-   [**IVssHardwareSnapshotProvider::OnLunEmpty**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384259)
-   [**IVssProviderCreateSnapshotSet::AbortSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384265)
-   [**IVssProviderCreateSnapshotSet::CommitSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384269)
-   [**IVssProviderCreateSnapshotSet::EndPrepareSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384272)
-   [**IVssProviderCreateSnapshotSet::PostCommitSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384277)
-   [**IVssProviderCreateSnapshotSet::PostFinalCommitSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384278)
-   [**IVssProviderCreateSnapshotSet::PreCommitSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384279)
-   [**IVssProviderCreateSnapshotSet::PreFinalCommitSnapshots**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384280)
-   [**IVssProviderNotifications::OnLoad**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384282)
-   [**IVssProviderNotifications::OnUnload**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384283)

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Developing VSS Hardware Providers](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381601)

Related technologies
--------------------

[Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832)

Operating system requirements
-----------------------------

Client

None supported

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using the command line:

-   Open the **Command Prompt** window and navigate to the directory.
-   Type **msbuild VssSampleProvider.sln**.

To build the sample using Visual Studio (preferred method):

-   Open File Explorer and navigate to the directory.
-   Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
-   In the **Build** menu, select **Build Solution**. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

**Note**  You must run this sample as an administrator.

1.  Install the virtual storage driver as follows:
    1.  Navigate to the Program Files (x86)\\Windows Kits\\8.0\\bin\\x86 directory in the Windows SDK. This directory contains virtualstoragevss.sys and vstorcontrol.exe.
    2.  Type **vstorcontrol.exe install** at the command prompt.

2.  Install the VSS sample provider as follows:
    1.  Copy the following files from the Program Files (x86)\\Windows Kits\\8.0\\bin\\x86 directory into the VssSampleProvider directory in the downloaded sample.
        -   VssSampleProvider.dll
        -   VstorInterface.dll
        -   install-sampleprovider.cmd
        -   uninstall-sampleprovider.cmd
        -   register\_app.vbs

    2.  In the VssSampleProvider directory, type **install-sampleprovider.cmd** at the command prompt.

3.  Create one or more virtual LUNs as follows:
    1.  At the command prompt, type **vstorcontrol.exe create fixeddisk -newimage C:\\disk1.image -size 20M -storid "VSS Sample HW Provider"**. This creates a virtual LUN whose storage identifier is "VSS Sample HW Provider". To create additional virtual LUNs, repeat this step.
        **Note**  The VSS sample provider will recognize a LUN only if "VSS Sample HW Provider" is a part of the storage identifier. For more information about the storage identifier, see [LUN-Resync with DiskShadow and Virtual Storage](%20http://go.microsoft.com/fwlink/p/?linkid=251516) on MSDN.
    2.  Use diskpart.exe or diskmgmt.msc to format the virtual disk and assign a drive letter to it.

4.  Run the sample provider by typing the following at the command prompt:

    **Run vshadow.exe -p -nw** *DriveLetter*

    where *DriveLetter* is the drive letter of the virtual LUN.

5.  To uninstall the VSS sample provider, do the following:
    1.  In the VssSampleProvider directory, type **uninstall-sampleprovider.cmd** at the command prompt.
    2.  Uninstall the virtual storage driver by typing **vstorcontrol.exe uninstall** at the command prompt.


