---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Cloud Mirror sample
urlFragment: cloudmirror-sample
description: Demonstrates how to get started writing a cloud files provider using the cloud files API. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Cloud Mirror sample

This sample shows how to get started writing a cloud files provider using the cloud files API.

This is the initial iteration, and as such it is an early preview, and far from final.

The following functionality is implemented:
* Declaring necessary Extensions and Capabilities in the *Package.appxmanifest*. Note that these declarations do not appear in the Visual Studio manifest editor.
* Registering/Unregistering a Sync Root which will show up in the Navigation Pane of Windows Explorer.
* Generating the initial placeholders in the Sync Root, using a physical "server" folder on the development machine as the fake cloud.
* Simulating Hydration of a file from a cloud service by slowly copying a file from a physical "server" folder on the development machine to a physical "client" folder on the development machine, including showing progress.
* Setting up custom states.
* Providing thumbnails for the file placeholders.
* adding a custom entry to the context menu when the user clicks on a file in the Sync Root.
* Supplying the URI of the cloud location of a file.

**Note**   The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

This sample was created for Windows 10 Version 1809 using Visual Studio 2017 using the Windows SDK 10.0.17763.0, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related topics

[**Documentation for this sample**](https://docs.microsoft.com/windows/desktop/cfapi/build-a-cloud-file-sync-engine)

[**Cloud files API**](https://docs.microsoft.com/en-us/windows/desktop/cfApi/cloud-files-api-portal)

[**UWP StorageProvider**](https://docs.microsoft.com/en-us/uwp/api/windows.storage.provider)


## Related technologies

[**Cloud files API**](https://docs.microsoft.com/en-us/windows/desktop/cfApi/cloud-files-api-portal)

[**UWP StorageProvider**](https://docs.microsoft.com/en-us/uwp/api/windows.storage.provider)

## Operating system requirements

### Client

Windows 10 Version 1809

## Build the sample

Currently the sample is configured for an x86 configuration. Change the solution platform to x86.

To build this sample, open the solution (*.sln*) file titled *CloudMirror.sln* from Visual Studio Professional 2017, Select **CloudMirrorPackage** as the startup project. go to **Build** \> **Build Solution** from the top menu after the sample has loaded.

**Warning**  This sample requires the Windows SDK 10.0.17763.0.

## Run the sample

1. To run this sample after building it, press **F5**. A **Directory Picker** dialog appears. 
1. Indicate the physical folder on your dev machine that holds a representation of the "cloud". After picking that folder, a second **Directory Picker** dialog appears. 
1. Indicate the location of your sync root (client), which is populated with placeholders of the files in the "cloud". Once you dismiss that picker, a console window appears with status messages, and the sync root appears in File Explorer.
1. Play around with the files in the sync root, making them available on the machine and freeing up space. Notice the custom state icons. Watch hydration show progress bars.
1. Press **CTRL**+**C** in the console window to gracefully exit.

The sample unregisters the sync root when it closes or crashes. This behavior is for demonstration purposes. A real-world provider would remain registered, so that when the user selects the sync root in File Explorer, the provider app restarts. Automatic restarting is not desirable for a demonstration, however.

**NOTE**: If you hydrated some files while testing and then shut down the sample, you should delete everything from the sync root folder before re-running the sample. Otherwise the sample will behave unpredictably.
