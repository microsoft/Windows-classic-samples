---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Per-window DPI Awareness sample
urlFragment: dpiawarenessperwindow-sample
description: Demonstrates different ways of dealing with screen DPI. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Per-window DPI Awareness sample

This sample shows how different version of Windows handle high DPI scaling of Win32 UI. This sample has two UI elements: 

1. A launcher window
2. A sample window

The launcher window enables you to launch a sample window in different DPI awareness modes, so that you can observe the different DPI-scaling capabilities of these different modes (and different versions of Windows). 

For more information about High DPI development on Windows, see [this](https://msdn.microsoft.com/en-us/library/windows/desktop/mt843498(v=vs.85).aspx) document. 

When you create a desktop application, you specify the level of DPI-awareness of the application, as one of the following modes:

* DPI-unaware
* System-DPI aware
* Per-Monitor DPI aware
* Per-Monitor DPI aware v2

Each of these modes results in different behavior for the application whenever the DPI/scale-factor changes for the display that the application is rendered on changes.
For example, this can happen because the application moves to a display with a different DPI value, when you connect via Remote Desktop from a device with a different DPI value, or because you change the scale factor in the Display Settings page.

Before the availability of the *SetThreadDpiAwarenessContext* API, a process could have only one DPI-awareness mode during its lifetime.
A developer would have to consider the tradeoffs of the different DPI-scaling modes across all of the UI in their application.
With the introduction of *SetThreadDpiAwarenessContext*, developers have more flexibility and can piece-meal update their app to handle DPI better. For example, they could focus their efforts on updating the most important parts of their application to handle DPI changes on a per-monitor basis, while leaving less important top-level windows to be scaled by Windows as if they had system DPI awareness or no DPI awareness at all.

**Note** *SetThreadDpiAwarenessContext* applies only to top-level windows.
By default, child windows receive the same DPI-awareness mode as their parents. You can use *SetThreadDpiHostingBehavior* to override this and have child windows run in a different scaling mode than that of their parent/host.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 10, version 1607 using Visual Studio 2015 Update 3, but in many cases it will run unaltered using later versions. 

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client
Windows 10, version 1803 or newer

### Windows SDK
10.0.17134.0 or newer

## Build the sample

1. Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2. Go to the directory where you downloaded the *SetThreadDpiAwarenessSample* sample and double-click its Microsoft Visual Studio Solution (*.sln*) file.

3. Press **F5** or use **Build** \> **Build Solution**.

## Run the sample

1. When the application is initialized, it shows a per-monitor DPI aware top-level window with text that indicates the DPI-awareness of the window and the DPI of the top-level window.

2. Moving the first top-level window to a display with a different DPI value will resize the window, resize its non-client area, and update the DPI value displayed in the top-level window.

3. When you click one of the buttons, it'll spawn a top-level window of that description. Inside this window, you'll see DPI-related information about the window, a button to open a system dialog with the same context as the window, and a child window that simulates a plugin by being sourced from another DLL and rendering as system DPI aware.

## Related concepts

[Writing DPI-Aware Desktop and Win32 Applications](https://msdn.microsoft.com/library/windows/desktop/dn469266.aspx)
