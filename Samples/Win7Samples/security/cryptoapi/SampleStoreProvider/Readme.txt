Copyright (c) Microsoft Corporation. All rights reserved.

Sample store provider
=====================
This sample shows how to implement a custom store provider by extending the CertOpenStore functionality. For detailed information on this, refer to the MSDN documentation on "Extending CertOpenStore functionality" here - http://msdn.microsoft.com/en-us/library/aa382403(vs.85).aspx 

IMPORTANT NOTE: 
==============
In order to store and manage certificates, Microsoft recommends applications to use the default system certificate stores available in Windows. We do not recommend creating custom certificate stores unless you need to store certificates in an external database outside of the certificate stores or need to implement semantics for certificate storage which are different from the system certificate stores.

This sample has two parts.

1. SampleStore project implements the custom store provider and creates a DLL.  
The sample does not implement all the functions that a store provider would need to implement. It shows how to implement some of the functions and includes stubs for the rest of the functions, which you would need to complete when you create your store provider. The sample creates a provider for an external store and for the purposes of the sample, certificates are stored in registry. You could instead use an external database for storage as per your requirements.

2. SampleStoreProvider project uses the sample store DLL. It shows how to add a certificate to this custom store.

Sample Language Implementations
===============================
C++

Files:
=====
SampleStore.sln
SampleStore.vcproj
SampleStore.cpp
SampleStore.def
SampleStoreProvider.cpp
SampleStoreProvider.vcproj

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib. Also specify the module definition file for the SampleStore project as samplestore.def.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild SampleStore.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, 
	a. First register the store provider DLL using "regsvr32.exe SampleStore.dll". You need to run this from an elevated command prompt.
	b. Then use "samplestoreprovider.exe /?" from the command prompt.
	SampleStoreProvider.exe [Options] {CertPath}
        Options:
                -p {PROVIDER}      : provider name (by default "TestExt")
                -s {STORENAME}     : store name (by default "TestStoreName")	
	{CertPath} is the path for the certificate file that will be added to the test store.
     3. To unregister the DLL use "regsvr32.exe /u SampleStore.dll". You need to run this from an elevated command prompt.

Comments:
This sample will create a test certificate store under HKEY_CURRENT_USER\Software\Microsoft\SystemCertificates. Once you run the sample, you will see a new registry key here with the test store name you provided, containing the test certificate. 

Note that: For 32-bit applications running on 64-bit machines, the store provider information is read from the WOW6432 node in the registry (specifically under HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Cryptography\OID\EncodingType 0\CryptDllOpenStoreProv).If you want to register the store provider for 32-bit applications on a 64-bit machine, build the DLL with a 32-bit as target platform in Visual Studio. When you register this DLL, the provider will be registered correctly in the WOW6432Node location
