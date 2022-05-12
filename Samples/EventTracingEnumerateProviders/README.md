---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Event tracing provider enumeration
urlFragment: eventtracing-enumerateproviders
description: Enumerates providers that have registered Event Tracing for Windows (ETW) decoding information
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

Event tracing provider enumeration sample
=========================================
This sample demonstrates how to get information about the providers that have
registered ETW decoding information on the system. This sample uses the
TdhEnumerateProviders and TdhEnumerateProvidersForDecodingSource APIs.

Prerequisites
=============
-   TdhEnumerateProviders requires Windows Vista or later.
-   TdhEnumerateProvidersForDecodingSource requires Windows 10 build 20348 or higher.
-   This sample requires the Windows Software Development Kit.

Sample language implementations
===============================
C++

To build the sample using Visual Studio (preferred method):
================================================================
     1. Open File Explorer and navigate to the directory.
     2. Double-click the icon for the .sln (solution) file to open the file in
     Visual Studio.
     3. In the Build menu, select Build Solution. The application will be
     built in the default \Debug or \Release directory.


To run the sample:
==================
     1. Press F5 in Visual Studio or later.
