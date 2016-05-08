Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a EOBO CMC request, enroll and save 
the output to a PFX file using certenroll API.

Files:
enrollEOBOCMC.cpp                   C++ source file
enrollEOBOCMC.sln                   Solution file
enrollEOBOCMC.vcproj                Project file
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
3.  Type "vcbuild enrollEOBOCMC.sln"

Usage:
enrollEOBOCMC <Template> <Requester> <FileOut> <Password> [<EATemplate>]

<Template>
Template to enroll

<Requester>
The domain user that the request enrolls on behalf of

<FileOut>
The output PFX filename

<Password>
Password used for the PFX output

[<EATemplate>] 
Optional
Template for EA (Enrollment Agent) Cert 
It should have EKU of Certificate Request Agent
Not required if there are already EA certs in personal store

Example:
enrollEOBOCMC User Domain\User pfx.out 1111 EnrollmentAgent