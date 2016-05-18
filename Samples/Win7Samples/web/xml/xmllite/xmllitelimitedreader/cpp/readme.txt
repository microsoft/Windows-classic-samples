//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
Overview
--------
This sample illustrates how to implement an XmlLite application that controls the reader memory
allocation. The reader memory allocation must be controlled to reduce denial of service threats
where there is a risk of documents being uploaded that may consume large amounts of memory. A
document can consume large amounts of memory in scenarios such as the following: 
- The document contains many nested elements.
- The document contains an element with many attributes.
- The document contains an element that has a very long name.

This sample does not limit memory by default. If you want to limit memory, you must modify the sample.
See the XmlLite documentation for more details.

XmlLite Documentation
---------------------
You can find the documentation for XmlLite in XMLLite.hxs in the Help directory of the SDK.

Requirements
------------
This sample requires the Microsoft Windows Software Development Kit (SDK) for Windows Vista and
.NET Framework 3.0. XmlLite samples require the XmlLite.h and XmlLite.lib files to build. XmlLite
samples require XmlLite.dll to run.

How to Build this Sample
------------------------
Build the sample using the latest Windows SDK. You can use Visual Studio to build the sample. If you do not
have Visual Studio installed, you can use the VCBuild utility to build the sample. In the sample directory,
enter the command VCBuild with no command line arguments.

How to Run this Sample
----------------------
Once you have built the sample, you can run it from the command line as follows:

release\XmlLiteLimitedReader verylongname.xml
