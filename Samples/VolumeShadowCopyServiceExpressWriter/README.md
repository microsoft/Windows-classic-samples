Volume shadow copy service express writer sample
================================================

This sample demonstrates the use of the [Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832) (VSS) API's express writer COM interfaces.

For more information about express writers, see [Volume Shadow Copy Service (VSS) Express Writers](http://go.microsoft.com/fwlink/p/?linkid=182214).

To test this sample, you'll need to use it together with a VSS requester such as the [BETest Tool](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530721).

This sample is written in C++ and requires some experience with COM.

This sample contains the following files:

-   ExpressW.sln
-   ExpressW.vcxproj
-   ExpressW.vcxproj.filters
-   helpers.cpp
-   helpers.h
-   main.cpp
-   readme.txt
-   stdafx.h

The following VSS API elements are used in the sample:

-   [**CreateVssExpressWriter**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405544)
-   [**IVssExpressWriter**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405596)
-   [**IVssExpressWriter::CreateMetadata**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405597)
-   [**IVssExpressWriter::Load**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405598)
-   [**IVssExpressWriter::Register**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405599)
-   [**IVssExpressWriter::Unregister**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd405600)
-   [**IVssCreateExpressWriterMetadata**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd765211)
-   [**IVssCreateExpressWriterMetadata::AddComponent**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd765212)
-   [**IVssCreateExpressWriterMetadata::AddFilesToFileGroup**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd765215)
-   [**IVssCreateExpressWriterMetadata::SaveAsXML**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd765216)
-   [**IVssCreateExpressWriterMetadata::SetRestoreMethod**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd765219)

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Volume Shadow Copy Service (VSS) Express Writers](http://go.microsoft.com/fwlink/p/?linkid=182214)

[BETest Tool](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530721)

Related technologies
--------------------

[Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using the command line:

-   Open the **Command Prompt** window and navigate to the ExpressW directory.
-   Type **msbuild ExpressW.sln**.

To build the sample using Visual Studio (preferred method):

-   Open File Explorer and navigate to the ExpressW directory.
-   Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
-   In the **Build** menu, select **Build Solution**. The application will be built in the default \\Debug or \\Release directory.

**Note**  It may be necessary to launch Visual Studio in

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable file, using the **Command Prompt** or **File Explorer**.
    **Note**  If you use the Command Prompt, you must run as administrator.
2.  Type the name of the executable file (expressw.exe by default) at the command prompt.

