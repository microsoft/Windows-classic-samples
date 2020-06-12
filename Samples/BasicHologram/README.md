---
page_type: sample
languages:
- cppwinrt
products:
- windows-api-win32
name: Basic hologram sample
urlFragment: basic-hologram
description: Demonstrates a simple hologram rendered by a Win32 process.
---

# Basic hologram sample

Demonstrates a simple hologram rendered by a Win32 process.

Specifically, this sample runs on Windows Mixed Reality desktop PCs and renders a spinning cube. You 
can interact with the cube by placing it in a new position, and a variety of input methods are allowed.
This sample works on PCs with headset devices attached, and it works on Microsoft HoloLens.

This sample supports one operational mode: upon app launch, a spinning cube will appear in front
of the user. To move the spinning cube, look somewhere else, and then use any of these input 
methods to place it in front of you:

* Air tap gesture, or "Select" speech command, when running on a Microsoft HoloLens
* Controller trigger button, or "Select" speech command, when running on a PC with headset
* The "A" button on an attached XBox 360 or XBox One controller
* Left mouse click

This sample also shows how to use the CommitDirect3D11DepthBuffer API. This API allows the app to 
submit a depth buffer for per-pixel image stabilization.

## Related topics

**Reference**

[**IHolographicSpaceInterop**](https://docs.microsoft.com/windows/desktop/api/holographicspaceinterop/nn-holographicspaceinterop-iholographicspaceinterop)
[**Windows.Perception**](https://msdn.microsoft.com/library/windows/apps/windows.perception.aspx)  
[**Windows.Perception.Spatial**](https://msdn.microsoft.com/library/windows/apps/windows.perception.spatial.aspx)  
[**Windows.Graphics.Holographic**](https://msdn.microsoft.com/library/windows/apps/windows.graphics.holographic.aspx)  

## Operating system requirements

**Client:** Windows 10 version 1803 or higher desktop PC

**Phone:** Not supported

## Build the sample

1. Start Microsoft Visual Studio 2017 and select **File** \> **Open** \> **Project/Solution**.
2. Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3. Press F7 or use **Build** \> **Build Solution** to build the sample.

## Run the sample


Before running the sample, ensure the Windows Mixed Reality headset is correctly plugged in, and that the Mixed Reality Portal is running.

**Note:** This code sample will also work with simulation mode enabled. For more information, see [Using the Windows Mixed Reality Simulator](https://docs.microsoft.com/en-us/windows/mixed-reality/using-the-windows-mixed-reality-simulator).

To debug the app and then run it, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.
