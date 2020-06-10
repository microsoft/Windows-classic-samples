---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Enumerate app packages sample
urlFragment: enumerate-app-packages-sample
description: Demonstrates how to enumerate all app packages that are provisioned on the machine. 
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Enumerate app packages

This sample shows how to enumerate all app packages that are provisioned on the machine.

Administrators and device manufacturers are able to provision an app package, using tools such as DISM or the *Add-ProvisionedAppPackage* cmdlet in Windows PowerShell. This causes the app package to be installed for each user on their first (or next) login.

The sample uses [**PackageManager.FindProvisionedPackages()**](https://docs.microsoft.com/en-us/uwp/api/Windows.Management.Deployment.PackageManager) to find all installed packages for all users.

**Warning**  This sample requires Microsoft Visual Studio 2019 Preview or a later version.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 10 build 18917 using Visual Studio 2019, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

## Related technologies

[App packaging and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446593)

## Operating system requirements

### Client

Windows 10, build 18917 (or later)

### Server

Windows Server 2012 R2

## Build the sample

### From the Command window

1.  Open a Command Prompt window.

2.  Go to the directory where you downloaded the *FindProvisionedPackages* sample.

3.  Run the following command.

    `msbuild FindProvisionedPackagesSample.sln`

### From Visual Studio

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory where you downloaded the *FindProvisionedPackages* sample and double-click its Microsoft Visual Studio Solution (.sln) file.

3.  Press **F7** or use **Build** \> **Build Solution**.

## Run the sample

1.  Open an elevated Command Prompt window.

2.  Go to the directory that contains *FindProvisionedPackagesSample.exe*.

3.  Run the following command.

    `FindProvisionedPackagesSample`


