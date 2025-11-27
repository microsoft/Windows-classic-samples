---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Transfer target sample
urlFragment: transfertarget-sample
description: Discovers entities that can receive a DataPackage and optionally transfers the data to them
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Transfer Target sample

This sample demonstrates the use of TransferTarget objects. A TransferTarget is an entity, such as an app, which can receive a DataPackage.

This sample shows how to

* Detect TransferTarget support on the system.
* Use a TransferTargetWatcher to discover TransferTarget objects that are compatible with a specific DataPackage.
* Configure the discovery of TransferTarget objects.
* Pass the DataPackage to one or more of the discovered TransferTarget objects.

An example of a TransferTarget is an app that can receive data via the Share contract.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697). To enable building and running all of the samples, make sure Visual Studio is configured with the Desktop development with C++, .NET desktop development, and Universal Windows Platform development workloads, as well as the the Windows App SDK C++ and C# Templates components. See [Set up your development environment](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=cs-vs-community%2Ccpp-vs-community%2Cvs-2022-17-1-a%2Cvs-2022-17-1-b#required-workloads-and-components) for more information.

## SDK requirements

This sample requires Windows SDK version 10.0.26100.7175 or higher.

## Operating system requirements

Windows 11 Build Major Version >= 26100 and Minor Version >= 7015.

## Building the sample

To build this sample:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

## Running the sample

Press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.
