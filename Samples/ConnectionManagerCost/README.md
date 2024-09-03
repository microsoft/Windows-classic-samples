---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Windows Connection Manager cost sample
urlFragment: connection-manager-wlan-cost
description: The sample demonstrates how to set and get cost for WLAN profiles using Windows Connection Manager (WCM) cost APIs.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Windows Connection Manager cost sample

The sample demonstrates how to set and get cost for WLAN profiles using Windows Connection Manager (WCM) cost APIs.

The APIs demonstrated in this sample are:

1. WcmSetProperty - to set cost or data plan status for a profile.
2. WcmQueryProperty - to query cost or data plan status info for a profile.

**Note** The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[Windows Connection Manager](https://learn.microsoft.com/en-us/windows/win32/wcm/windows-connection-manager-portal)

## System requirements

- Requires Windows SDK 10.0.22621.0 to build and Windows 8 to run.

## Build the sample

To build the sample using Visual Studio (preferred method):

1. Open **Windows Explorer** and navigate to the **\\cpp** directory.
2. Double-click the icon for the **.sln** (solution) file to open the file in Visual Studio.
3. In the **Build** menu, select **Build Solution**. The application will be built in the default **\\Debug** or **\\Release** directory.

To build the sample using the command prompt:

1. Open the **Command Prompt** window and navigate to the directory containing the sample for a specific language.
2. Type **msbuild wcmcostsample**.

## Run the sample

To run the sample:

1. Navigate to the directory that contains **wcmcostsample.exe**, using the **command prompt** or **Windows Explorer**.
2. Type **wcmcostsample.exe** at the **command prompt**, or double-click the icon for **wcmcostsample** to launch it from **Windows Explorer**.
