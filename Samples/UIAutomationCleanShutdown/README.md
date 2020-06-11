UI Automation clean shutdown sample
===================================

This sample has two pieces that work together to show the correct way to clean up and shut down a Microsoft UI Automation control that is hosted in a DLL. The first piece is a DLL that contains a simple control with a basic UI Automation provider. The second piece is an application that hosts the control.

When the host application starts, it displays the **Load DLL** button. When you press the button, the host application loads the DLL and uses it to create and display the control. Also, the text of the button changes to **Unload DLL**. When you press the button again, the host application destroys the control and unloads the DLL.

When the control is destroyed, the provider calls the [**UiaDisconnectProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437312) function. If any external UI Automation client applications were connected to the control, the call to [**UiaDisconnectProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437312) ensures that the clients release their references so the DLL can unload safely.

A good way to see the sample work is to connect a UI Automation client application after you press the **Load DLL** button. Two good client applications are Narrator (the screen reader application included with Windows) and Inspect (an accessibility testing tool included in the Windows Software Development Kit (SDK)).

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[UI Automation](http://msdn.microsoft.com/en-us/library/windows/desktop/ee684009)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample. Go to the C++ directory and double-click the Visual Studio Solution (.sln) file.
3.  In Solution Explorer, select UiaCleanShutdownControl to build the simple control DLL, or UiaCleanShutdownHost to build the host application. (You'll need to build both pieces to use this sample.)
4.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

To run the sample after building it, follow these steps:

1.  Copy both pieces (the control DLL and the host application) to the same location. You'll find the control DLL at \<*install\_root*\>\\UI Automation Clean Shutdown Sample\\C++\\UiaCleanShutdownControl\\Debug\\UiaCleanShutdownControl.dll, and the host application at \<*install\_root*\>\\UI Automation Clean Shutdown Sample\\C++\\UiaCleanShutdownHost\\Debug\\UiaCleanShutdownHost.exe.
2.  Navigate to the location where you copied both pieces of the sample.
3.  Type UiaCleanShutdownHost.exe at the command line, or double-click the icon for UiaCleanShutdownHost.exe to launch it from Windows Explorer.

