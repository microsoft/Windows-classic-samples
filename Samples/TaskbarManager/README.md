---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Taskbar pinning sample
urlFragment: taskbarmanager-sample
description: Demonstrates how to pin an app or secondary tile to the taskbar
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

TaskbarManager desktop samples
==================

This sample demonstrates how to use the TaskbarManager API from a desktop app (with or without package identity) to request pinning your app or a secondary tile (deep link) for your app to the taskbar. This sample includes sample apps showing how to detect support for and use this pinning functionality from C++ and C# and from a packaged and unpackaged desktop app.

See [Pin your app to the taskbar](https://learn.microsoft.com/en-us/windows/apps/design/shell/pin-to-taskbar) and [TaskbarManager Class](https://learn.microsoft.com/en-us/uwp/api/windows.ui.shell.taskbarmanager) for more information and detailed documentation.

**Warning**  At the time of publishing, this sample requires a recent Windows Insider Preview SDK, 23516 or later, *and* a recent Windows build (Windows Insider Preview Dev channel 23516 or later). Check [Windows Insider Preview Downloads](https://www.microsoft.com/en-us/software-download/windowsinsiderpreviewSDK) for the latest preview SDK.

**Warning**   At the time of publishing, using the TaskbarManager API from a desktop app requires unlocking the appropriate Limited Access Feature. You will not be able to meaningfully use these samples without obtaining an unlock token for the feature and providing it in the appropriate locations (LafData.h/.cs) according to the instructions you receive in the response to your request. For more information or to request an unlock token, contact [Microsoft Support](https://support.serviceshub.microsoft.com/supportforbusiness/create?sapId=d15d3aa2-0512-7cb8-1df9-86221f5cbfde).

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697). To enable building and running all of the samples, make sure Visual Studio is configured with the Desktop development with C++, .NET desktop development, and Universal Windows Platform development workloads, as well as the the Windows App SDK C++ and C# Templates components. See [https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=cs-vs-community%2Ccpp-vs-community%2Cvs-2022-17-1-a%2Cvs-2022-17-1-b#required-workloads-and-components](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=cs-vs-community%2Ccpp-vs-community%2Cvs-2022-17-1-a%2Cvs-2022-17-1-b#required-workloads-and-components) for more information.

Build the sample
----------------

To build this sample:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

First, ensure the desired sample project is configured as the startup project: Right-click on the project in the solution view and select "Set as Startup Project". You can choose to run the appropriate packaged/unpackaged C++/C# sample as desired.

Once this is configured, to debug the app and then run it, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.
