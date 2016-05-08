Copyright (c) Microsoft Corporation. All rights reserved.

Acquire a private key associated with a certificate and use it for signing
==========================================================================
This example shows how to acquire private key associated with a certificate, determine its type (CAPI or CNG) and used it to sign hashed message. 
In addition it demonstrates creating hash using CAPI or CNG APIs.
Please note: Even though this sample shows CNG hash signed by CNG key and 
CAPI hash signed by CAPI key, it is possible to use CNG key to sign CAPI hash

Sample Language Implementations
===============================
C++

Files:
=====
CertSign.sln
certSign.vcproj
Sign.cpp
CngSigningCert.pfx
CapiSigningCert.pfx

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib and ncrypt.lib

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild CertSign.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "CertSign.exe /?" from the command prompt.
	certsign.exe [Options] {SIGN|VERIFY} InputFile SignatureFile
        Options:
                -s {STORENAME}   : store name, (by default "MY")
                -n {SubjectName} : Certificate CN to search for, (by default "Test")
                -h {HashAlgName} : hash algorithm name, (by default "SHA1")
Comments:
=========
The sample includes two PFX files which contain test certificates you can use to test out the sample. The PFX password is "test".








