Sub-Process DPI Awareness Sample
===================================================

This sample shows how to use the SetThreadDpiAwarenessContext API to have different top-level windows within a single desktop application process run with different dots-per-inch (DPI) awareness modes. It also shows how to use the EnableNonClientDpiAwareness API to enable the non-client area of a top-level window scale dynamically for DPI.

When creating a desktop application, you have the ability to specify the level of DPI-awareness of the application (DPI-unaware, System-DPI aware, Per-Monitor DPI aware). Each of these modes results in different behavior for the application whenever the dots-per-inch of the display that the application is rendered on changes (moving the application to a display with a different DPI value, connecting via Remote Desktop from a device with a different DPI value, or if the user changes the display-scale factor in the Display Settings page). Before the availability of SetThreadDpiAwarenessContext a process could only have one DPI-awareness mode during its lifetime. This meant that a developer would have to choose between the tradeoffs of the different DPI-scaling modes across all of the UI in their application. With the introduction of SetThreadDpiAwarenessContext developers have more flexibility and can, for example, focus their efforts on updating the most-important parts of their application's UI to handle DPI changes natively (the most important top-level window could be configured to run with per-monitor-DPI awareness and be designed to update its layout when a DPI change occurs) while having Windows handle the scaling for less-important top-level windows (by having these top-level windows use system-DPI-awareness or be DPI-unaware). 

**Note** SetThreadDpiAwarenessContext supports specifying a DPI-awareness mode on top-level windows only. It does not support setting a different DPI-awareness mode for child windows (in other words: you can't have part of your window drawn natively for the DPI of the display it's on while Windows scales child windows)

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for the Windows 10, version 1607 using Visual Studio 2015 Update 3, but in many cases it will run unaltered using later versions. 

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related Concepts
--------------------

[Writing DPI-Aware Desktop and Win32 Applications](https://msdn.microsoft.com/en-us/library/windows/desktop/dn469266(v=vs.85).aspx)

Operating system requirements
-----------------------------

**Client**
Windows 10, version 1607 or newer

**Windows SDK**
10.0.14393.0 or newer

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory where you downloaded the SetThreadDpiAwarenessSample sample and double-click its Microsoft Visual Studio Solution (.sln) file.

3.  Press F5 or use **Build** \> **Build Solution**.

Run the sample
--------------

1.  When the application is initialized a per-monitor DPI aware top-level window will be shown with text that indicates the DPI-awareness of the window and the DPI of the top-level window.

2.  Moving the first top-level window to a display with a different DPI value will result in the window resizing, its non-client area resizing, and the DPI value displayed in the top-level window being updated.

3.  From the file menu you can create a second top-level window. This top-level window runs with system-DPI awareness and will be DPI scaled by Windows. Notice that the DPI value that this window shows does not change even when the window is moved to a display with a different DPI.

