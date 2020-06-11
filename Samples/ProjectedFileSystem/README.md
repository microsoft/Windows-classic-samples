---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: RefFS sample
urlFragment: ProjectedFileSystem
description: "Sample provider for the Windows Projected File System (ProjFS)."
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# RegFS sample provider

This sample is an example provider for the Windows Projected File System (ProjFS).

The Registry File System (RegFS) sample creates a virtual projection of the local machine's registry into a file system folder using the [Windows Projected File System (ProjFS)](https://docs.microsoft.com/en-us/windows/desktop/projfs/projected-file-system).  The sample uses ProjFS to map registry keys to folders, and registry values to files.  This version of RegFS allows a user to explore the registry namespace and read registry values, but it does not allow modifications.  This means that it does not allow deleting or renaming the projected files, and it also blocks modifying their contents.

**Note** The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the Windows Dev Center. This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 10, version 1809 using Visual Studio 2017, but in many cases it will run unaltered using later versions.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client

Windows 10, version 1809 or newer

### Server 

Windows Server 2019

### Windows SDK 

10.0.17763.0 or newer

## Enable ProjFS

ProjFS is an Optional Component available in the Windows 10 October 2018 Update (Windows 10, version 1809), Windows Server 2019, and later versions of Windows.

To use PowerShell to enable ProjFS use the `Enable-WindowsOptionalFeature` cmdlet in an elevated PowerShell window:

```PowerShell
Enable-WindowsOptionalFeature -Online -FeatureName Client-ProjFS -NoRestart
```

Reboot the computer if the `Enable-WindowsOptionalFeature` reports `RestartNeeded: True`.

## Build the sample

1. Start Visual Studio and select **File > Open > Project/Solution...**.

1. Go to the directory where you downloaded the RegFS sample and double-click its Microsoft Visual Studio Solution (*.sln*) file.

1. Press **F5** or use **Build > Build Solution**

## Run the sample

1. Start a RegFS virtualization instance.

   Run `regfs.exe [virtualization root]`.  For example, `regfs.exe c:\regfsRoot`. *regfs.exe* will create the virtualization root folder if it does not already exist.

1. Open another command-line window and perform operations in the virtualization root.

   For example, if your command-line window is CMD you might try these operations:
   * `cd /d c:\regfsRoot` to change to the virtualization root.
   * `dir` to view all the registry subkeys and values under the current path.
   * `cd [subkey name]` to change the current path to a particular registry key. For example, `cd HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PrjFlt`.
   * `type [value name]` to read a registry value.  For example, `type ImagePath`.

1. To stop the provider and exit the sample, press **Enter**.
   You can restart the sample to resume virtualization.
   If you are finished with the sample, you can manually delete the virtualization root folder.
