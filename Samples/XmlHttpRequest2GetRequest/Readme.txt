IXMLHTTPRequest2 HTTP GET Sample
================================
This sample demonstrates how to use IXMLHTTPRequest2 interface to asynchronously sending
HTTP requests and receiving HTTP responses.

The interfaces demonstrated in this sample are:
     1. IXMLHTTPRequest2
     2. IXMLHTTPRequest2Callback

Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

Files
=====
XMLHttpRequestGet.cpp
     This file contains wmain function.
XMLHTTPRequest2Callback.h
     This file contains the definition of class CXMLHTTPRequest2Callback.
XMLHTTPRequest2Callback.cpp
     This file contains the implementation of class CXMLHTTPRequest2Callback. It demonstrates the core call back functions for HTTP request.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory containing the sample for a specific language.
     2. Type "MSBuild.exe XMLHttpRequestGet.sln".

To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open File Explorer and navigate to the directory containing the sample for CPP language.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt.
     2. Type "XMLHttpRequestGet.exe <url>" at the command line.
