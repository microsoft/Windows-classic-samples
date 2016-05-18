Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a 2-layer nested CMC request and enroll
using certenroll API.

Files:
enrollNestedCMC.cpp                 C++ source file
enrollNestedCMC.sln                 Solution file
enrollNestedCMC.vcproj              Project file
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
3.  Type "vcbuild enrollNestedCMC.sln"

Usage:
enrollNestedCMC <FileIn> <FileOut> [<SigningTemplate>]

<FileIn>
The input request file. It should be a template based CMC request

<FileOut>
The output filename to save the response

[<SigningTemplate>]
Optional
Template that supports signing
Not required if there are already signing certs in personal store

Example:
enrollNestedCMC Cmc.req Response.out User