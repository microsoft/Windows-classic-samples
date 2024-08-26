---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Virtualization-based security (VBS) enclave sample
urlFragment: VbsEnclave
description: "Shows how to create and run code in a Virtualization-based security (VBS) enclave."
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

Virtualization-based security (VBS) Enclave Sample Code
=======================

This sample demonstrates the lifecycle of a VBS enclave including making function calls into the enclave. It also serves as a starting point for new enclave projects since it has all the MSVC configuration for enclaves inside the `.vcxproj` files.

Requirements
------------

- Windows 11
- Visual Studio 2022 version 17.9 or above, with Build Tools installed
- Windows SDK version 10.0.22621.3233 or above


Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the `Samples/VbsEnclave` directory, and double-click the Microsoft Visual Studio Solution (.sln) file titled `Enclave Sample.sln`.

3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

*Note*: The enclave won't run until it is signed. Please refer the VBS Enclaves Development Guide for the details.

VBS Enclaves Development Guide
------------------------------

The development guide provides insights on writing, compiling and debugging enclaves using this sample code as an example.

It is available at: [aka.ms/VBSEnclavesDevGuide](https://aka.ms/VBSEnclavesDevGuide)
