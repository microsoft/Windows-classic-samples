Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a simple template based user request
and enroll using certenroll API.

Files:
enrollSimpleUserCert.cpp            C++ source file
enrollSimpleUserCert.sln            Solution file
enrollSimpleUserCert.vcproj         Project file
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
3.  Type "vcbuild enrollSimpleUserCert.sln"

Usage:
enrollSimpleUserCert <Template> <SubjectName> <KeyLength>

<Template>  
User template name to enroll

<SubjectName>
Subject name

<KeyLength>
Private key length

Example: 
enrollSimpleUserCert User "CN=foo,OU=bar,DC=com" 1024