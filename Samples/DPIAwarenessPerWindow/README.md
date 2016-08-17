Per-window DPI Awareness Sample
===================================================

This sample shows how to use the SetThreadDpiAwarenessContext function
to assign different dots-per-inch (DPI) awareness modes to different
top-level windows within a single desktop application process.
It also shows how to use the EnableNonClientDpiAwareness function
to scale the non-client area of a top-level window dynamically for DPI.

When you create a desktop application,
you can specify the level of DPI-awareness of the application:
DPI-unaware, System-DPI aware, and Per-Monitor DPI aware.
Each of these modes results in different behavior for the application
whenever the dots-per-inch changes for the display
that the application is rendered on.
For example, this can happen because the application moves
to a display with a different DPI value,
or because the user connects
via Remote Desktop from a device with a different DPI value,
or because the user changes
the display-scale factor in the Display Settings page.

Before the availability of SetThreadDpiAwarenessContext,
a process could have only one DPI-awareness mode during its lifetime.
A developer would have to consider the tradeoffs
of the different DPI-scaling modes
across all of the UI in their application.
With the introduction of SetThreadDpiAwarenessContext,
developers have more flexibility and can,
for example, focus their efforts on updating the
most important parts of their application
to handle DPI changes on a per-monitor basis
and update their layout when a DPI change occurs,
while leaving less important top-level windows
to be scaled by Windows as if they had
system DPI awareness or no DPI awareness at all.

**Note** SetThreadDpiAwarenessContext applies
only to top-level windows.
Child windows receive the same DPI-awareness mode as their parents.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 10, version 1607 using Visual Studio 2015 Update 3, but in many cases it will run unaltered using later versions. 

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related Concepts
--------------------

[Writing DPI-Aware Desktop and Win32 Applications](https://msdn.microsoft.com/library/windows/desktop/dn469266.aspx)

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

1.  When the application is initialized,
it shows a per-monitor DPI aware top-level window
with text that indicates the DPI-awareness of the window
and the DPI of the top-level window.

2.  Moving the first top-level window to a display
with a different DPI value will resize the window,
resize its non-client area,
and update the DPI value displayed in the top-level window.

3.  From the file menu you can create a second top-level window.
This top-level window runs with system-DPI awareness
and will be scaled by Windows.
Notice that the DPI value that this window shows
does not change even when the window is moved
to a display with a different DPI.

