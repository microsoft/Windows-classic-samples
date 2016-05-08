Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a PKCS10 request with custom
extensions and enroll using certenroll API.

Files:
enrollCustomPKCS10.cpp              C++ source file
enrollCustomPKCS10.sln              Solution file
enrollCustomPKCS10.vcproj           Project file
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
3.  Type "vcbuild enrollCustomPKCS10.sln"

Usage:
enrollCustomPKCS10 <SubjectName> <RFC822Name> <EKU>

<SubjectName>
Subject name

<RFC822Name>
RFC822 Name to put in Subject Alternative Name

<EKU>
Enhanced Key Usage OID. For example, 
1.3.6.1.5.5.7.3.2 is the OID for Client Authentication.

Example:
enrollCustomPKCS10 "CN=foo,OU=bar,DC=com" User@Domain.com 1.3.6.1.5.5.7.3.2
