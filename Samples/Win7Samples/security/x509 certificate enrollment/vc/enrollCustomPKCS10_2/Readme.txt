Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a PKCS10 request, collect
ICspInformations for use and enroll using certenroll API.

Files:
enrollCustomPKCS10_2.cpp            C++ source file
enrollCustomPKCS10_2.sln            Solution file
enrollCustomPKCS10_2.vcproj         Project file
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
3.  Type "vcbuild enrollCustomPKCS10_2.sln"

Usage:
enrollCustomPKCS10_2 <Template> <ProviderName>

<Template>
Template to enroll

<ProviderName>
Cryptographic service provider name

Example:
enrollCustomPKCS10_2 User "Microsoft Enhanced Cryptographic Provider v1.0"