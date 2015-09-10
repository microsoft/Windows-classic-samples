Windows Security Center API sample
==================================



Demonstrates how to query the Windows Security Center for product information for any products registered with the Security Center that are antivirus, antispyware, or firewall products.



**Important**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU). It doesn't compile in Microsoft Visual Studio Express 2013 for Windows

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

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

1.  Open a Command Prompt window.

2.  Go to the directory that contains WscApiSample.exe.

3.  Run the following command:

    **WscApiSample** [**-av**|**-as**|**-fw**]**[-av|-as|-fw]** where **-av** queries for antivirus programs, **-as** queries for antispyware programs, and **-fw** queries for firewall programs.

    For example, **WscApiSample -av** would provide information about antivirus programs that are registered with the Windows Security Center.


