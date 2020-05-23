---
page_type: sample
languages:
- cpp
- csharp
products:
- windows-api-win32
name: App package removal sample
urlFragment: app-package-removal
description: Demonstrates how to remove an app package from the system using the Package deployment API.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# App package removal sample

This sample shows how to remove an app package from the system using the [Package deployment API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh994436).

Users acquire your app as an app package. Windows uses the information in an app package to install the app on a per-user basis, and ensure that all traces of the app are gone from the device after all users who installed the app uninstall it. Each package consists of the files that constitute the app, along with a package manifest file that describes the app to Windows.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU); it doesn't compile with Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Related technologies

[App packaging and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446593)

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

### From the Command window

1.  Open a Command Prompt window.

2.  Go to the directory where you downloaded the *RemovePackage* sample.

3.  Run the following command.

    ```msbuild RemovePackageSample.sln```

### From Visual Studio

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory where you downloaded the *RemovePackage* sample and select its Microsoft Visual Studio Solution (*.sln*) file.

3.  Press **F7** (or **F6** for Visual Studio 2013) or use **Build** \> **Build Solution**.

## Run the sample

1.  Open a Command Prompt window.

2.  Go to the directory that contains *RemovePackageSample.exe*.

3.  Run the following command:

    ```RemovePackageSample package-fullname```

    For example, ```RemovePackageSample testapp\_1.0.0.0\_neutral\_en-us\_ab1c2d3efghij```.

## Related topics

### Samples

[Add app package sample](http://go.microsoft.com/fwlink/p/?linkid=236968)

### Concepts

[App packages and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh464929)

### Reference

[Windows.Management.Deployment.PackageManager](http://msdn.microsoft.com/en-us/library/windows/desktop/br240960)

