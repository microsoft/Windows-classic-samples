Copyright (c) Microsoft Corporation. All rights reserved.

Creating a self-signed certificate
====================================
This sample code shows how to create a certificate that has a private key associated with it.
For the purposes of the sample, we create a self-signed certificate.
The sample code also illustrates how to encode RDNs in the subject name of a certificate using the CertStrToName API.
Note that this sample requires Windows Vista or higher to run as it depends on the CNG API set which was introduced in Windows Vista.

APIs:
====
This example illustrates the use of the following APIs

1. CertOpenStore: This function opens a certificate store by using a specified store provider type
2. CertStrToName: This converts a string to an encoded certificate name
3. CertCreateSelfSignCertificate: This function creates a self signed certificate
4. CertCloseStore: This function closes a certificate store handle

Sample Language Implementations
===============================
C++

Files:
=====
CreateCertificate.sln
CreateCertificate.vcproj
CreateCert.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib and ncrypt.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild CreateCertificate.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "CreateCertificate.exe /?" from the command prompt.
	CreateCertificate.exe [Options] SubjectName
        Options:
                -c {Container}     : container name (by default "SAMPLE")
                -s {STORENAME}     : store name (by default "MY")
                -l {Bits}                       : key size
                -k {CNGAlgName}    : key algorithm name (by default "RSA")
                -h {CNGAlgName}    : hash algorithm name (by default "SHA1")
	
	Specify the SubjectName with RDNs:
	For example: CreateCertificate.exe "CN=TEST,OU=TESTOU"




