---
page_type: sample
languages:
- cpp
- cppwinrt
products:
- windows-api-win32
description: "Create a compositor application that can present Direct3D content to a specialized monitor."
urlFragment: DisplayCoreCustomCompositor
---

# Windows.Devices.Display.Core Custom Compositor Sample

This sample demonstrates using the Windows.Devices.Display.Core APIs to create a compositor application that can present Direct3D content to a specialized monitor (such as a custom VR/AR headset).

## Operating system requirements

Windows 10 version 1903

## Build the sample

To build this sample, open the solution (.sln) file titled DisplayCoreCustomCompositor.sln from Visual Studio 2019 or later versions. Press F7 or go to Build-\>Build Solution from the top menu after the sample has loaded.

## Run the sample

Running this sample requires having a physical monitor that adheres to the [EDID Extension for HMDs and Specialized Displays](https://docs.microsoft.com/en-us/windows-hardware/drivers/display/specialized-monitors-edid-extension). Connect a compatible monitor before attempting to run the sample.

To run this sample after building it, press F5 (run with debugging enabled) or Ctrl-F5 (run without debugging enabled) from Visual Studio.