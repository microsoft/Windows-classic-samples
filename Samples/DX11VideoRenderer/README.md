DirectX video rendering sample
==============================

This sample shows how to create a media sink that renders video output to a window using DirectX 11.

Specifically, this sample shows how to:

-   Decode the video using the Media Foundation APIs
-   Render the decoded video using the DirectX 11 APIs
-   Output the video stream to multi-monitor displays

For more info about the concepts and APIs demonstrated in this sample, see the following topics:

-   [Direct3D 11 graphics APIs](http://msdn.microsoft.com/en-us/library/windows/desktop/ff476080)
-   [Media Foundation media APIs](http://msdn.microsoft.com/en-us/library/windows/desktop/ms694197)
-   [DirectX Graphics Interface (DXGI) APIs](http://msdn.microsoft.com/en-us/library/windows/desktop/bb205169)

**Note**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and will not build in Microsoft Visual Studio Express 2013 for Windows .

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[Media Foundation](http://msdn.microsoft.com/en-us/library/windows/desktop/ms694197)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build this sample:

1.  Open the solutions (.sln) file titled DX11VideoRenderer.sln from Visual Studio. You'll find the solutions file in \<*install\_root*\>\\DX11VideoRenderer\\C++.
2.  After the sample has loaded, press the F7 (or F6 for Visual Studio 2013) key or select **Build Solution** from the **Build** menu.

Run the sample
--------------

1. Register the DLL by running the command `regsvr32 DX11VideoRenderer.dll` from an elevated command prompt.
2. Run [the `topoedit.exe` program from the Windows SDK](https://docs.microsoft.com/en-us/windows/desktop/medfound/topoedit).
3. From the Topology menu, select "Add DX11 Video Renderer."
4. When you are finished using the sample, unregister the DLL by running the command `regsvr32 /u DX11VideoRenderer.dll` from an elevated command prompt.
