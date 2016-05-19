Copyright (c) Microsoft Corporation. All rights reserved.

Encrypting Data and creating an enveloped message
=================================================
This sample code shows how to encrypt and decrypt a PKCS7 (CMS) message using the CryptEncryptMessage and CryptDecryptMessage APIs.

APIs:

This example illustrates the use of the following APIs

1. CryptEncryptMessage: This function encrypts and encodes a message
2. CryptDecryptMessage: This function decodes and decrypts a message
3. CertOpenStore: This function opens a certificate store by using a specified store provider type
4. CertFindCertificateInStore: This function finds the first or next certificate context in a certificate store that matches search criteria 
5. CertCloseStore: This function closes a certificate store handle

Sample Language Implementations
===============================
C++

Files:
=====
cms_encrypt.sln
cms_encrypt.vcproj
cms_encrypt.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild cms_encrypt.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, first use "cms_encrypt.exe /?" from the command prompt.
	cms_encrypt.exe [Options] {COMMAND}
   	Options:
     	-s {STORENAME}   : store name, (by default MY)
     	-n {SubjectName} : Recepient certificate's CN to search for.
    	                   (by default "Test")
     	-a {CNGAlgName}  : Encryption algorithm, (by default AES128)
     	-k {KeySize}     : Encryption key size in bits, (by default 128)
   	COMMANDS:
     	ENCRYPT {inputfile} {outputfile}
        	              | Encrypt message
     	DECRYPT {inputfile} {outputfile}
                	      | Decrypt message

Note:
While using ENCRYPT option, the inputfile is the file to be encrypted and the output file is the encrypted output.
While using DECRYPT option, the inputfile is the encrypted file and the output file is the decrypted output. 









