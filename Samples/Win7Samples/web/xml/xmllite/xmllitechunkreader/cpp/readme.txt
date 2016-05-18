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
When XML documents are very large, reading with chunking allows you to write an application that can read
large documents while maintaining a predictable memory usage profile. Chunking is a technique where instead
of getting the value for an element or an attribute all at once, the module using XmlLite has the option to
retrieve a maximum amount of data in a single method call. If the element or attribute has more data than the
specified maximum, chunking allows the data to be retrieved in multiple sequential calls to a method. This
sample illustrates how to implement an XmlLite application that reads an XML document using the chunking
functionality.

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

release\XmlLiteChunkReader chunks.xml
