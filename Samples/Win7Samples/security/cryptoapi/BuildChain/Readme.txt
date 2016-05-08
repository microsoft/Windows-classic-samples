Copyright (c) Microsoft Corporation. All rights reserved.

Build and verify a certificate chain
====================================
Demonstrates how to validate a certificate by building and verifying the certificate chain.
For the purposes of the sample, we look for a test certificate in the user's personal certificate store and build a chain for it.
The sample code also illustrates how to check for chain validation errors using the CertVerifyCertificateChainPolicy API to perform basic certificate verification checks.

APIs
====
This example illustrates the use of the following APIs

1. CertOpenStore: This function opens a certificate store by using a specified store provider type
2. CertFindCertificateInStore: This function finds the first or next certificate context in a certificate store that matches search criteria
3. CertGetCertificateChain: This function builds a certificate chain context starting from an end certificate and going back, if possible, to a trusted root certificate.
4. CertVerifyCertificateChainPolicy: This function checks a certificate chain to verify its validity, including its compliance with any specified validity policy criteria


Sample Language Implementations
===============================
C++

Files:
=====
BuildChain.sln
BuildChain.vcproj
BuildChain.cpp
sampletest.pfx 

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild BuildChain.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "BuildChain.exe /?" from the command prompt.
	BuildChain.exe [Options] SubjectName
        Options:
                -f 0xHHHHHHHH    : CertGetCertificateChain flags
                -s STORENAME     : store name, (by default MY)

Comments:
=========
This sample includes a test certificate chain which you can use for testing purpose. When you install the test certificates, you will be prompted to install the root certificate to the user store. The PFX password is "sampletest".
It is not recommended to install root certificates to user root store, but you may do so for this sample for testing purposes.
If you use the test certificate, you may see an error: "Error: 0x80092012 (-2146885614) The revocation function was unable to check revocation for the certificate." This is because the test certificate does not contain a CRL distribution point or OCSP AIA extension. Certificates, in practice, generally contain these pointers to revocation information and the sample shows how to check revocation using the appropriate flags for the APIs.
