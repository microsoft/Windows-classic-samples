VShadow Volume Shadow Copy Service sample
=========================================

VShadow is a command-line tool that you can use to create and manage volume shadow copies. It is also a sample that demonstrates the use of the [Volume Shadow Copy Service](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968832) (VSS) COM API.

For more information about the VSS tool and its command-line options, see [VShadow Tool and Sample](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530725) and [VShadow Tool Examples](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530726).

This sample is written in C++ and requires some experience with COM.

This sample contains the following files:

-   break.cpp
-   create.cpp
-   delete.cpp
-   expose.cpp
-   macros.h
-   query.cpp
-   readme.html
-   readme.txt
-   revert.cpp
-   select.cpp
-   shadow.cpp
-   shadow.h
-   stdafx.cpp
-   stdafx.h
-   tracing.cpp
-   tracing.h
-   util.h
-   vshadow.rc
-   vshadow.sln
-   vshadow.vcxproj
-   vshadow.vcxproj.filters
-   vssclient.cpp
-   vssclient.h
-   writer.cpp
-   writer.h

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[VShadow Tool and Sample](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530725)

[VShadow Tool Examples](http://msdn.microsoft.com/en-us/library/windows/desktop/bb530726)

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

-   Open the **Command Prompt** window and navigate to the directory.
-   Type **msbuild Vshadow.sln**.

To build the sample using Visual Studio (preferred method):

-   Open File Explorer and navigate to the directory.
-   Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
-   In the **Build** menu, select **Build Solution**. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

To run the sample:

1.  Navigate to the directory that contains the new executable file, using the **Command Prompt** or **File Explorer**.
    **Note**  If you use the Command Prompt, you must run as administrator.
2.  Type the name of the executable file (vshadow.exe by default) at the command prompt.

