---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Basic appp bundle extract sample
urlFragment: extract-app-bundle
description: Demonstrates how to extract info about a bundle package using the Packaging API.
---

# Extract app bundle contents sample

This sample shows how to extract info about a bundle package using the [Packaging API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446766).

The sample covers these tasks:

-   Use [IAppxBundleFactory::CreateBundleReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280279) to create a bundle reader
-   Use [IAppxBundleReader::GetFootprintFile](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280298) to extract footprint files from the bundle reader
-   Use [IAppxBundleReader::GetPayloadPackages](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280301) to get [IAppxFilesEnumerator](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446685) to iterate through the list of payload packages. Then, use [IAppxFile](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446683) to get info about each package.

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

[IAppxBundleReader](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280296)
