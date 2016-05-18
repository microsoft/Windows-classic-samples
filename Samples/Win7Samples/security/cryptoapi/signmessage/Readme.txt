Copyright (c) Microsoft Corporation. All rights reserved.

Signing and Signature Verification
==================================
This sample shows how to sign a message and verify the signature. 

This uses the CryptSignMessage and CryptVerifySignature APIs. It illustrates how to obtain the OID for an algorithm from its name and then use it in the signing parameters. 

CryptSignMessage API requires the hash algorithm used for signing to be provided as one of the Signing Parameters. You need to provide the Object Identifier (OID) for the algorithm. Given the name of the algorithm, the CryptFindOIDInfo API can be used to obtain the OID. This can then be used for signing as shown in the sample code.

APIs:
=====
This example illustrates the use of the following APIs,

1. CertOpenStore: This opens a certificate store
2. CertFindCertificateInStore: This function selects the signer certificate from the store.
3. CryptFindOIDInfo: This function maps the algorithm identifier to the corresponding OID.
4. CryptSignMessage: This function creates a hash of the specified content, signs the hash,
	and then encodes both the original message content and the signed hash.
5. CryptVerifyMessageSignature: This function verifies a signed message's signature.

Sample Language Implementations
===============================
C++

Files:
=====
cms_sign.sln
cms_sign.vcproj
cms_sign.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild cms_sign.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "cms_sign.exe /?" from the command prompt.
	cms_sign.exe [Options] {COMMAND}
	    Options:
	      -s {STORENAME}   : store name, (by default MY)
	      -n {SubjectName} : Recepient certificate's CN to search for.
        	                (by default "Test")
	      -a {CNGAlgName}  : Hash algorithm, (by default SHA1)
	    COMMANDS:
	      SIGN {inputfile} {outputfile}
	                       | Sign message
	      VERIFY {inputfile}
        	               | Verify message
