Windows Vista Certenroll C++ Sample

Description:
This sample demonstrates how to create a custom CMC request and enroll 
using certenroll API.

Files:
enrollCustomCMC.cpp                 C++ source file
enrollCustomCMC.sln                 Solution file
enrollCustomCMC.vcproj              Project file
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
3.  Type "vcbuild enrollCustomCMC.sln"

Usage:
enrollCustomCMC <Name> <Value> <DNS> <EKU>

<Name>
The name in a name/value pair

<Value>
The value in a name/value pair

<DNS>
DNS name to put in Subject Alternative Name

<EKU>
Enhanced Key Usage OID. For example, 
1.3.6.1.5.5.7.3.1 is the OID for Server Authentication.

Example:
enrollCustomCMC Name Value www.adatum.com 1.3.6.1.5.5.7.3.1
