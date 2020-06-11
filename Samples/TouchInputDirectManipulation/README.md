---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Process touch input with Direct Manipulation sample
urlFragment: process-touch-input
description: Demonstrates how to process touch input with the Direct Manipulation.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Process touch input with Direct Manipulation sample

This sample shows how to process touch input with the [Direct Manipulation](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446969) APIs and support user interactions with smooth animations and feedback behaviors.Use these APIs to optimize UI response and reduce latency through off-thread input processing and predictive output based on the rendering time of the compositor.

Using two viewports, one nested in the other, this sample demonstrates:

-   Creating and configuring a viewport
-   Associating viewports with primary content
-   Establishing correct chaining and parent promotion behaviors between nested viewports
-   Registering event handlers to listen to viewport updates
-   Using [Direct Manipulation](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446969) with [DirectComposition](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437371)

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related technologies

[Direct Manipulation](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446969), [DirectComposition](http://msdn.microsoft.com/en-us/library/windows/desktop/hh437371)

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

1.  Start Microsoft Visual Studio Express 2013 for Windows and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (*.sln*) file.
3.  Press **F7** or use **Build** \> **Build Solution** to build the sample.

## Run the sample

To debug the app and then run it, press **F5** or use **Debug** \> **Start Debugging**. To run the app without debugging, press **Ctrl**+**F5** or use **Debug** \> **Start Without Debugging**.

## Related topics

[User Interaction](http://msdn.microsoft.com/en-us/library/windows/desktop/ff657750)

[Graphics and Gaming](http://msdn.microsoft.com/en-us/library/windows/desktop/ee663279)