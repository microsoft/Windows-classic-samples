Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to Create a CMC request from a public key 
and enroll using certenroll API.

Files:
enrollFromPublicKey.cpp             C++ source file
enrollFromPublicKey.sln             Solution file
enrollFromPublicKey.vcproj          Project file
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
3.  Type "vcbuild enrollFromPublicKey.sln"

Usage:
enrollFromPublicKey <Template> <FileOut> [<SigningTemplate>]

<Template>
Template to enroll

<FileOut>
The filename to save the response

[<SigningTemplate>]
Optional
Template that supports signing
Not required if there are already signing certs in personal store

Example:
enrollFromPublicKey User Response.out User