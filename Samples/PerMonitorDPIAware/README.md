Per-Monitor Aware WPF Sample\>
==============================

This sample demonstrates updating a Windows Presentation Foundation (WPF) application to be per-monitor DPI-aware.

The sample consists of two projects:

-   **NativeHelpers.vcxproj** The native helper project that implements the core functionality to make a WPF application per-monitor DPI-aware
-   **WPFApplication.csproj** A sample WPF application that inherits from the PerMonitorDPIWindow base class and showcases how the application window resizes when the user moves the window to another monitor with a different DPI or when the user changes the DPI by adjusting the Display slider in Control Panel.

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
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

To debug the app and then run it, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

