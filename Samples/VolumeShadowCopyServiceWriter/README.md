Volume Shadow Copy Service writer sample
========================================

This sample demonstrates the use of the [Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832) (VSS) COM interfaces to create a VSS writer.

For more information about VSS writers, see [Writers](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384993).

To test this sample, you'll need to use it together with a VSS requester such as the [BETest Tool](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530721).

This sample is written in C++ and requires some experience with COM.

This sample contains the following files:

-   main.cpp
-   main.h
-   SampleWriter.sln
-   SampleWriter.vcxproj
-   SampleWriter.vcxproj.filters
-   swriter.cpp
-   swriter.h
-   stdafx.h

The following VSS API elements are used in the sample:

-   [**CVssWriter::Initialize**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381543)
-   [**CVssWriter::Subscribe**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa381588)
-   [**IVssCreateWriterMetadata::AddComponent**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa383595)
-   [**IVssCreateWriterMetadata::AddFilesToFileGroup**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa383871)
-   [**IVssCreateWriterMetadata::SetRestoreMethod**](http://msdn.microsoft.com/en-us/library/windows/desktop/aa383909)

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Writers](http://msdn.microsoft.com/en-us/library/windows/desktop/aa384993)

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

-   Open the **Command Prompt** window and navigate to the SampleWriter directory.
-   Type **msbuild SampleWriter.sln**.

To build the sample using Visual Studio (preferred method):

-   Open File Explorer and navigate to the SampleWriter directory.
-   Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
-   In the **Build** menu, select **Build Solution**. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

Note that the sample must be built for the native operating system. A 64-bit operating system cannot load a 32-bit writer.

To run the sample:

1.  Navigate to the directory that contains the new executable file, using the **Command Prompt** or **File Explorer**.
    **Note**  If you use the Command Prompt, you must run as administrator.
2.  Type the name of the executable file (samplewriter.exe by default) at the command prompt.

