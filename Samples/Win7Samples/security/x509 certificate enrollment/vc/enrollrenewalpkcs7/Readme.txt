Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a Pkcs7 renewal request and enroll
using certenroll API.

Files:
enrollRenewalPKCS7.cpp              C++ source file
enrollRenewalPKCS7.sln              Solution file
enrollRenewalPKCS7.vcproj           Project file
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
3.  Type "vcbuild enrollRenewalPKCS7.sln"

Usage:
enrollRenewalPKCS7 <Template>

<Template>
Template to renew

Example:
enrollRenewalPKCS7 User