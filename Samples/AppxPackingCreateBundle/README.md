---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Create app bundle sample
urlFragment: create-app-bundle
description: Demonstrates how to create an app bundle using the Packaging API.
---

# Create app bundle sample

This sample shows you how to create an app bundle using the [Packaging API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh446766).

The sample covers these tasks:

-   Use [IAppxBundleFactory::CreateBundleWriter](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280280) to create a bundle writer.
-   Use [IAppxBundleWriter::AddPayloadPackage](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280303) to add the payload packages to the bundle.
-   Create an input stream and use [IAppxBundleWriter::Close](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280304) to flush the stream.

## Operating system requirements

### Client

Windows 8.1

### Server

Windows Server 2012 R2

## Build the sample

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (*.sln*) file.
3.  Press **F7** or use **Build** \> **Build Solution** to build the sample.

## Run the sample

To debug the app and then run it, press **F5** or use **Debug** \> **Start Debugging**. To run the app without debugging, press **Ctrl**+**F5** or use **Debug** \> **Start Without Debugging**.

## Related topics

### Reference

[IAppxBundleFactory::CreateBundleWriter](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280280)

[IAppxBundleWriter](http://msdn.microsoft.com/en-us/library/windows/desktop/dn280302)
