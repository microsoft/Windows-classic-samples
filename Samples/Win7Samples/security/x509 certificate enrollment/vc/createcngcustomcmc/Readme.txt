Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a custom CNG CMC request using
certenroll API.

Files:
createCNGCustomCMC.cpp              C++ source file
createCNGCustomCMC.sln              Solution file
createCNGCustomCMC.vcproj           Project file
Readme.txt                          This file

Platform
This sample requires Windows Vista.

Build using Visual Studio:
1. Open Windows Explorer and navigate to the directory
2. Double-click the icon for the .sln file
3. In the Build menu, select Build Solution
 
Build using Windows SDK:
1.  From the Start->All Programs menu choose Microsoft Windows SDK -> CMD Shell
2.  In the WinSDK CMD Shell, navigate to this directory
3.  Type "vcbuild createCNGCustomCMC.sln"

Usage:
createCNGCustomCMC <ProviderName> <AlgName> <HashAlgName> 
            <FileOut> [AlternateSignature]

<ProviderName>
Cryptographic service provider name

<AlgName>
Asymmetric algorithm name

<HashAlgName> 
Hash algorithm name

<FileOut>
Output filename to save the encoded CMC request

[AlternateSignature]
Optional
If "AlternateSignature" is specified, we will set the
AlternateSignature flag for the CMC requst, otherwise, not. 

Example:
createCNGCustomCMC "Microsoft Software Key Storage Provider" ECDSA_P521
            SHA1 Cmc.out AlternateSignature
        