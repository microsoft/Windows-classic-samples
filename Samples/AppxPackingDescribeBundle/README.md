---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Query app bundle info sample
urlFragment: query-app-bundle-info
description: Demonstrates how to query info about a bundle package using the Packaging API.
---

# Query app bundle info sample

This sample shows how to query info about a bundle package using the [Packaging API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446766).

The sample covers these tasks:

-   Use [IAppxBundleFactory::CreateBundleReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280279) to create a bundle reader, and then [IAppxBundleReader::GetManifest](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280299) to get [IAppxBundleManifestReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280292) for the bundle manifest reader.
-   Use [IAppxBundleManifestReader::GetPackageId](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280293) to get an [IAppxManifestPackageId](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446717) to read package identity info.
-   Use [IAppxBundleManifestReader::GetPackageInfoItems](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280294) to get [IAppxBundleManifestPackageInfoEnumerator](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280282) to iterate through the list of payload packages that are described in a bundle package manifest. Then, use [IAppxBundleManifestPackageInfo](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280281) to get info about each package.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and select the Visual Studio Solution (*.sln*) file.
3.  Press **F7** or use **Build** \> **Build Solution** to build the sample.

## Run the sample

To debug the app and then run it, press **F5** or use **Debug** \> **Start Debugging**. To run the app without debugging, press **Ctrl**+**F5** or use **Debug** \> **Start Without Debugging**.

## Related topics

### Reference

[IAppxBundleManifestReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280292)

[IAppxBundleReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280296)