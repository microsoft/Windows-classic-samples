---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Tab Integration sample
urlFragment: apptabsintegration-sample
description: Demonstrates how tabbed apps can expose their tabs to the system.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Tab Integration sample

This sample shows how a tabbed app can expose its tabs to the system.

* Detecting whether WindowTab support exists on the system.
* Obtaining the WindowTabManager for a window.
* Associating each app tab with a WindowTab object.
* Updating the list of WindowTabs when the app tabs change.
* Updating the the WindowTab as the app tab's state changes.
* Setting the WindowTab as active when the user changes tabs.
* Providing thumbnails for tabs.
* Responding to system-initiated tab switches.
* Responding to system-initiated tab tear-out.

**Note**   The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

This sample was created for Windows 11 using features added in the May 2023 release, using Visual Studio and the Windows SDK 10.0.22621.1778, but it should also work with later versions of Windows and the Windows SDK.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[**WindowTabManager**](https://learn.microsoft.com/en-us/uwp/api/windows.ui.shell.windowtabmanager)

## Operating system requirements

Windows 11 May 2023 release

## Build the sample

* Install the Windows SDK version 10.0.22621.1778 or higher.
* Open the solution (*.sln*) file titled *AppTabsIntegration.sln* from Visual Studio.
* **Important**: Add a reference to the Desktop Extension SDK version 10.0.22621.1778 or higher. (If you forget, the build will remind you.)
* Press Ctrl+Shift+B or select **Build** \> **Build Solution**.

## Run the sample

Press F5 or select Debug \> Start Debugging. To run the sample without debugging, press Ctrl+F5 or select Debug \> Start Without Debugging. 

Things to try with the sample:

* Use the + button to create multiple tabs.
* Alt+Tab integration: Requires "Show tabs from apps when snapping or pressing Alt+Tab" in Settings to show recent tabs.
    * Press Alt+Tab and observe that additional tabs show up in the Alt+Tab list in addition to the active tab. The app itself is represented by the active tab. The most recently used non-active tabs show up as separate windows in the Alt+Tab UI.
    * Select one of the non-active tabs. Observe that the app switches tabs to the tab you selected.
* Snap Assist integration: Requires Alt+Tab integration, as well as "Snap windows" and "When I snap a window, suggest what I can snap next to it" to be enabled.
    * Snap the app window to trigger Snap Assist. Observe that the most recently used non-active tabs show up as snap options.
    * Select one of the non-active tabs of the window that you snapped. Observe that the tab is "torn out" and placed next to the snapped window.
* Select a tab, change its title, and change its color. Repeat the above steps to observe that the changes are reflected in the Alt+Tab and Snap Assist UI.
