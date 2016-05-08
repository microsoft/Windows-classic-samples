Copyright (c) Microsoft Corporation. All rights reserved.

Peer Trust
==========
This sample code shows chain building for a certificate in a Trusted People store. 

The Trusted People store contains certificates which are explicitly trusted by the user. These incldue certificates for Outlook, Outlook Express and EFS on Microsoft Windows XP and later. This sample code illustrates how to build a chain for a certificate in this store and verify that chain complies with policy. 

A thing to note in this sample code is the use of the CERT_CHAIN_ENABLE_PEER_TRUST flag. Since peer trust implies explicit trust by the user, it is not mandatory to build a chain to trusted root. If this flag is enabled, this chain building is not done, thereby optimizing the validation process.

APIs:
====
This example illustrates the use of the following APIs,

1. CertOpenStore: This function opens a certificate store by using a specified store provider type
2. CertFindCertificateInStore: This function finds the first or next certificate context in a certificate store that matches search criteria
3. CertCreateCertificateChainEngine: This function creates a new, nondefault chain engine for an application.
4. CertGetCertificateChain: This function builds a certificate chain context starting from an end certificate and going back, if possible, to a trusted root certificate.
5. CertVerifyCertificateChainPolicy: This function checks a certificate chain to verify its validity, including its compliance with any specified validity policy criteria

Sample Language Implementations
===============================
C++

Files:
=====
peertrust.sln
peertrust.vcproj
peertrust.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild peertrust.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "peertrust.exe /?" from the command prompt.
	peertrust.exe [Options]
        Options:
          -s {STORENAME}     : store name (by default "TrustedPeople")
          -fe 0xHHHHHHHH     : CertCreateCertificateChainEngine flags
          -fc 0xHHHHHHHH     : CertGetCertificateChain flags
