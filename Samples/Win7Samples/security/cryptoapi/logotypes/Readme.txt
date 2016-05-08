Copyright (c) Microsoft Corporation. All rights reserved.

Retrieval of Logotype or Biometric Information in a Certificate
===============================================================
This sample code shows how to use the CertRetrieveLogoOrBiometricInfo API to retrieve the issuer logo from a certificate that has a logotype extension.

CertRetrieveLogoOrBiometricInfo is an API (available in Windows Vista and later) to support the two new X.509 extensions namely the BioMetricInfo extension and the Logotype extension.

The BioMetricInfo extension (IETF RFC 3739) supports the addition of a signature or a pictorial representation of the human holder of the certificate.
The Logotype extension (IETF RFC 3709) adds support for the addition of organizational pictorial representations in certificates. 

APIs:
====
This example illustrates the use of the following APIs,

1. CertOpenStore: This opens a certificate store.
2. CertFindCertificateInStore: This function selects the certificate from the store.
3. CertRetrieveLogoOrBiometricInfo: This function retrieves the logotype or biometric information in the certificate.

Sample Language Implementations
===============================
C++

Files:
=====
logotypes.sln
logotypes.vcproj
logotypes.cpp
TestCertificate.cer

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild logotypes.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, use "logotypes.exe" from the command prompt.

Comments:
=========
This example retrieves the issuer logo from a certificate. To successfully run this sample, you will need a certificate with a logotype extension. The certificate subject name should be inserted in the code in the "SUBJECT_NAME" definition. This certificate must be present in the Intermediate CA store. If it is present in a separate store, that should be specified in the call to CertOpenStore.

The sample includes a reference test certificate which has a logotype extension and can be used to test this sample.

If the retrieval is unsuccessful, the sample highlights the possible cause of error. This include common scenarios such as absence of the logotype information in the certificate or incorrect hash value.




