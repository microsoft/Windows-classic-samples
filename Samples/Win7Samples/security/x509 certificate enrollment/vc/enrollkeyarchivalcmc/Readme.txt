Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a key archival CMC request and enroll
using certenroll API.

Files:
enrollKeyArchivalCMC.cpp            C++ source file
enrollKeyArchivalCMC.sln            Solution file
enrollKeyArchivalCMC.vcproj         Project file
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
3.  Type "vcbuild enrollKeyArchivalCMC.sln"

Usage:
enrollKeyArchivalCMC <Template>

<Template>
Template that supports key archival

Example:
enrollKeyArchivalCMC KeyArchival