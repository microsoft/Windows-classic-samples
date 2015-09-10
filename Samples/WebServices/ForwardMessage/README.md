Windows Web Services message forwarding sample
==============================================

This sample demonstrates how to use the [Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435) to forward a message.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[**WsCreateChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430495)

[**WsOpenChannel**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430574)

[**WsInitializeMessage**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430568)

[**WsWriteMessageStart**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430660)

[**WsCreateListener**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430500)

[**WsOpenListener**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430575)

Related technologies
--------------------

[Windows Web Services API](http://msdn.microsoft.com/en-us/library/windows/desktop/dd430435)

Operating system requirements
-----------------------------

Client

Windows 7

Server

Windows Server 2008 R2

Build the sample
----------------

To build the sample using Visual Studio (preferred method):

1.  Open **Windows Explorer** and navigate to the directory.
2.  Double-click the icon for the **ForwardMessage.sln** (solution) file to open the file in Visual Studio.
3.  Change the active solution platform to the platform you want in the **Configuration Manager** found in the **Build** menu.
4.  In the **Build** menu, click **Build Solution**. The app is built in the default **\\Debug** or **\\Release** directory.

To build the sample using the command line:

1.  Open the **Command Prompt** window and navigate to the directory.
2.  Type **msbuild ForwardMessage.sln**.

Run the sample
--------------

To run this sample after building it, press **F5** (run with debugging enabled) or **Ctrl-F5** (run without debugging enabled) from Visual Studio 8.1 or later versions of Visual Studio and Windows (any SKU).

