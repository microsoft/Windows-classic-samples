---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Query app package and app manifest sample
urlFragment: query-app-package-manifest
description: Demonstrates how to query info about an app package using the app.
---

# Query app package and app manifest sample

This sample shows how to query info about an app package using the app [Packaging API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446766).

Users acquire your app as an app package. Windows uses the information in an app package to install the app on a per-user basis, and ensure that all traces of the app are gone from the device after all users who installed the app uninstall it. Each package consists of the files that constitute the app, along with a package manifest file that describes the app to Windows.

The sample covers the following tasks:

-   Use [IAppxFactory::CreatePackageReader](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446677) to create a package manifest reader.
-   Use [IAppxManifestReader::GetPackageId](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446745) to get an [**IAppxManifestPackageId**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446717) to read package identity info.
-   Use [IAppxManifestReader::GetProperties](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446748) to get an [**IAppxManifestProperties**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446731) to read package properties.

The sample covers this new task for Windows 8.1:

-   Use [IAppxManifestReader2::GetQualifiedResources](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280313) to get [**IAppxManifestQualifiedResourcesEnumerator**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280306) to iterate through all qualified resources defined in the manifest, not just language resources. Then, use [**IAppxManifestQualifiedResource**](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280305) to get info about each resource.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU); it doesn't compile with Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

### From the Command window

1.  Open a Command window.

2.  Go to the directory where you downloaded the DescribeAppx sample.

3.  Run the following command:

    ```msbuild DescribeAppx.sln```

### From Visual Studio

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory where you downloaded the DescribeAppx sample and select its Microsoft Visual Studio Solution (*.sln*) file.

3.  Press **F7** (or **F6** for Visual Studio 2013) or use **Build** \> **Build Solution**.

## Run the sample

1.  Open a Command window.

2.  Go to the directory that contains *DescribeAppx.exe*.

3.  Run the following command:

    ```DescribeAppx appPackage.appx```

    For testing purposes, you can specify *Data\\SamplePackage.appx* as the package.

## Related topics

### Tasks

[Quickstart: Read app package manifest info](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446622)

### Concepts

[App packages and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh464929)

### Reference

[IAppxBlockMapReader](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446651)

[IAppxManifestReader](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446737)

[IAppxPackageReader](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446756)

## Related technologies

[App packaging and deployment](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446593)
