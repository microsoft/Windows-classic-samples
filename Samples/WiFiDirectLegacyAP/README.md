---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Wi-Fi Direct Legacy Connection C++ WRL Demo sample
urlFragment: wifi-direct-legacy
description: Demonstrates how to enumerate all app packages installed on the system, and enumerate every user that installed each package.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Wi-Fi Direct Legacy Connection C++ WRL Demo

This sample is a simple desktop console application that uses WRL to demonstrate the Wi-Fi Direct legacy AP WinRT API from a desktop application.

Developers of desktop applications can use this sample to see how to replace the deprecated WlanHostedNetwork* API's with the new WinRT API's without modifying the application to become a Universal Windows Application. These API's let an application start a Wi-Fi Direct Group Owner (GO) that acts as an Access Point (AP). This allows devices that do not support Wi-Fi Direct to connect to the Windows device running this application and communicate over TCP/UDP. The API's allow the developer to optionally specify an SSID and passphrase, or use randomly generated ones.

The sample is organized up into the following files:

- **WlanHostedNetworkWinRT.cpp/h** : This contains the code that uses the API in the `WlanHostedNetworkHelper` class. There is also a `IWlanHostedNetworkListener` interface that can be used by another application to receive notifications from events in the Wi-Fi Direct API. This part may be used as is or modified to fit your application needs.
- **SimpleConsole.cpp/h** : This is a simple console using iostreams to take command line input and start or stop the Wi-Fi Direct legacy AP. It implements the `IWlanHostedNetworkListener` to handle receiving messages from the API.
- **WiFiDirectLegacyAPDemo.cpp** : Main entry point that starts the simple console.

**Note** This sample requires Windows 10 to execute, as it uses new API's. It also requires a Wi-Fi Card and Driver that supports Wi-Fi Direct. These API's **do not** support cross-connectivity so clients connecting to this device will not be able to use it for Internet access.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client 

Windows 10

### Server 

Windows 10

### Phone  

Windows 10

## Build the sample

1. Start Microsoft Visual Studio 2015 and select **File** \> **Open** \> **Project/Solution**.
2. Go to the directory to which you unzipped the sample. Double-click the Visual Studio 2015 Solution (*.sln*) file. 
3. Press **Ctrl**+**Shift**+**B**, or select **Build** \> **Build Solution**. 

## Run the sample

To run this sample after building it, press **F5** (run with debugging enabled) or **Ctrl**-**F5** (run without debugging enabled) from Visual Studio Express 2015 for Windows 10 or later versions of Visual Studio and Windows (any SKU). (Or select the corresponding options from the Debug menu.)

## Related topics

[Wi-Fi Direct WinRT API](https://msdn.microsoft.com/en-us/library/windows.devices.wifidirect.aspx)

[WlanHostedNetwork* API (deprecated in Windows 10)](https://msdn.microsoft.com/en-us/library/windows/desktop/dd815243.aspx)

[More information on MSDN](https://msdn.microsoft.com/en-us/library/windows/hardware/mt244265(v=vs.85).aspx)
