Copyright (c) Microsoft Corporation. All rights reserved.

Retrieve and embed an OCSP response
===================================
This sample shows how to staple an OCSP response as a property on a certificate and use it for validation.
The sample also shows how to retrieve the OCSP response from the revocation information in a chain and store it to a file.
The sample uses a certificate from a specified certificate store on the machine.

Sample Language Implementations
===============================
C++

Files:
=====
OCSP_response.sln
OCSP_response.vcproj
main.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild OCSP_response.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "OCSP_Response.exe /?" from the command prompt.
	OCSP_Response.exe [Options] {SubjectName} {OcspRespFile}
	  Options:
	   -s STORENAME : store name, (by default MY)
	   -staple      : staple certificate with {OcspRespFile} for verification,
        	        : otherwise OCSP response will be stored to {OcspRespFile}.

