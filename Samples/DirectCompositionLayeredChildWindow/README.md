DirectComposition layered child window sample
=============================================

This sample demonstrates how to use the Microsoft DirectComposition API to animate the bitmap of a layered child window.It consists of a simple video player that lets you apply animated 2-D transforms to the video window as a video plays.

Specifically, this sample shows how to:

-   Create layered child windows by applying the **WS\_EX\_LAYERED** extended window style
-   Animate layered child windows and apply animated 2-D transforms (translate, rotate, skew, and scale)
-   Use the window cloaking feature to hide a layered child window's "real" window bitmap while DirectComposition animates the visual representation of the window

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and won't compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Animation](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437348)

[How to animate the bitmap of a layered child window](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437378)

Related technologies
--------------------

[DirectComposition](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437371)

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
2.  Go to the directory named for the sample. Go to the cpp directory and double-click the Visual Studio Solution (.sln) file.
3.  Press F6 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

To run this sample after building it, use Windows Explorer to go to the installation folder for this sample and run DirectComposition\_LayeredChildWindow.exe from the *\<install\_root\>*\\DirectComposition layered child window sample\\C++\\Debug folder.

To run this sample from Microsoft Visual Studio, press the F5 key to run with debugging enabled, or Ctrl+F5 to run without debugging enabled. Alternatively, select **Start Debugging** or **Start Without Debugging** from the **Debug** menu.

To see this sample in action, you need to load a media file for the sample to play in its video window. You'll find a demo media file called vc1 in the *\<install\_root\>*\\DirectComposition layered child window sample\\C++\\media folder.

