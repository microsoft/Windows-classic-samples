Copyright (c) Microsoft Corporation. All rights reserved.

Providing exclusive trust anchors for certificate validation
============================================================
This sample code shows how to pass in exclusive trust anchors for certificate validation. 
By default, certificate validation on Windows uses the trusted self-signed certificates from the Trusted Root Certification Authorities store as trust anchors. 
Windows 7 provides a way for calling applications to pass in exclusive trust anchors which are independent of the system trust anchors. This can be done by creating a non-default certificate chain engine as shown in the sample.

APIs:
====
This example illustrates the use of the following APIs

1. CertOpenStore: This function opens a certificate store by using a specified store provider type
2. CertFindCertificateInStore: This function finds the first or next certificate context in a certificate store that matches search criteria
3. CryptQueryObject: This function retrieves information about the contents of a cryptography API object, such as a certificate, a certificate revocation list, or a certificate trust list. 
4. CertCreateCertificateChainEngine: This function creates a new, nondefault chain engine for an application.
5. CertGetCertificateChain: This function builds a certificate chain context starting from an end certificate and going back, if possible, to a trusted root certificate.
6. CertVerifyCertificateChainPolicy: This function checks a certificate chain to verify its validity, including its compliance with any specified validity policy criteria

NOTE: THIS SAMPLE WILL NOT COMPILE ON WINDOWS VISTA AS THIS USES API CHANGES AVAILABLE ONLY IN WINDOWS 7.

Sample Language Implementations
===============================
C++

Files:
=====
ExclusiveTrust.sln
ExclusiveTrust.vcproj
ExclusiveTrust.cpp
sampletest.sst
testEE.cer 

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild ExclusiveTrust.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "ExclusiveTrust.exe /?" from the command prompt.
        ExclusiveTrust.exe [Options] {StoreFile} [EndCertFile]
        Options:
          -fe 0xHHHHHHHH     : CertCreateCertificateChainEngine flags
          -fc 0xHHHHHHHH     : CertGetCertificateChain flags
          -p                 : Peer Trust

To test using the test certificates included in the sample, run ExclusiveTrust.exe sampletest.sst testEE.cer

Comments:
=========
This sample includes a test certificate chain which you can use for testing purpose. There are two files included in the sample - sampletest.sst (which is a serialized store file containing the exclusive root and intermediate CA certificate) and testEE.cer(a test end-entity certificate). 

