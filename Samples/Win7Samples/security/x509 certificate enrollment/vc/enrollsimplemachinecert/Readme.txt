Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a simple template based machine request
and enroll using certenroll API.

Files:
enrollSimpleMachineCert.cpp         C++ source file
enrollSimpleMachineCert.sln         Solution file
enrollSimpleMachineCert.vcproj      Project file
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
3.  Type "vcbuild enrollSimpleMachineCert.sln"

Usage:
enrollSimpleMachineCert <Template> <CertFriendlyName> <CertDescription>

<Template>  
Machine template name to enroll

<CertFriendlyName>
Certificate friendly name

<CertDescription>
Certificate description

Example: 
enrollSimpleMachineCert Machine "Machine Cert" "Simple Machine Cert"