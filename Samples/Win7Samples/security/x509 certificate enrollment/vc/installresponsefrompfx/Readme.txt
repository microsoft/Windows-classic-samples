Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to install response from a PFX file using 
certenroll API.

Files:
installResponseFromPFX.cpp          C++ source file
installResponseFromPFX.sln          Solution file
installResponseFromPFX.vcproj       Project file
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
3.  Type "vcbuild installResponseFromPFX.sln"

Usage:
installResponseFromPFX <FileIn> <Password>

<FileIn>
The input PFX file name

<Password>
Password for the PFX file

Example:
installResponseFromPFX Cert.pfx 1111