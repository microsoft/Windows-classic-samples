DirectComposition effects with Direct2D bitmap content sample
=============================================================

This sample shows how to use Microsoft DirectComposition to apply animations and effects to visuals that have Direct2D bitmap content.

The sample creates two DirectComposition visual trees. The visual tree on the right consists of a single root visual, and the visual tree on the left consists of a root visual and four child visuals. Clicking the left mouse button applies animated 3-D transformation effects to the visual trees, including rotation, scaling, perspective, and translation transformations. In addition, animation is applied to the right visual's Opacity property to change the visual's opacity from transparent to opaque. Pressing the 1-4 number keys changes the color of the right visual.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Effects](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437372)

[How to apply effects](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437379)

[How to apply animations](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437377)

Related technologies
--------------------

[DirectComposition](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437371) , [Direct2D](http://msdn.microsoft.com/en-us/library/windows/desktop/dd370990), [Direct3D](http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466)

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

To run this sample after building it, go to the installation folder for this sample with Windows Explorer and run DirectComposition\_Effects.exe from the *\<install\_root\>*\\DCompEffectsD2DSDK\\C++\\Debug folder.

To run this sample from Microsoft Visual Studio, press the F5 key to run with debugging enabled, or Ctrl+F5 to run without debugging enabled. Alternatively, select **Start Debugging** or **Start Without Debugging** from the **Debug** menu.

