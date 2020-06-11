---
page_type: sample
languages:
- cpp
- csharp
products:
- windows-api-win32
name: Enumerate all app packages sample
urlFragment: enumerate-all-app-packages
description: Demonstrates how to enumerate all app packages installed on the system, and enumerate every user that has installed each package.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Enumerate app packages sample

This sample shows how to enumerate all app packages installed on the system, and enumerate every user that has installed each package.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

### From the Command window

1.  Open a Command Prompt window.

2.  Go to the directory where you downloaded the *FindPackagesWithPackageTypes* sample.

3.  Run the following command.

    ```msbuild FindPackagesWithPackageTypesSample.sln```

### From Visual Studio

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory where you downloaded the *FindPackagesWithPackageTypes* sample and select its Microsoft Visual Studio Solution (*.sln*) file.

3.  Press **F7** or use **Build** \> **Build Solution**.

## Run the sample

1.  Open a Command Prompt window.

2.  Go to the directory that contains *FindPackagesWithPackageTypesSample.exe*.

3.  Run the following command.

    ```FindPackagesWithPackageTypesSample```

## Related topics

[App packaging and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446593)

### Tasks

[How to inventory packages](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446620)

### Concepts

[App packages and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh464929)

### Reference

[Windows.Management.Deployment.PackageManager](http://msdn.microsoft.com/en-us/library/windows/desktop/br240960)

