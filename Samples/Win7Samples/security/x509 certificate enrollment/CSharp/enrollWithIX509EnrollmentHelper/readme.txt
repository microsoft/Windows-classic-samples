Windows 7 X509CertificateEnrollment C# Sample

Sample name: enrollWithIX509EnrollmentHelper

Description:
This sample demonstrates how to use the Windows 7 new http protocol to 
enroll a certificate by calling the IX509EnrollmentHelper::AddEnrollmentServer 
and IX509Enrollment2::Enroll methods. The purpose of the call to the
IX509EnrollmentHelper::AddEnrollmentServer is to cache the authentication
credential to enrollment server in Windows vault.

This sample does not support certificate authentication type

Files:
enrollWithIX509EnrollmentHelper.cs		C# source file
enrollWithIX509EnrollmentHelper.sln		Solution file
enrollWithIX509EnrollmentHelper.csproj		Project file
readme.txt					This file

Platform:
This sample requires Windows 7.

Build with Visual Studio 2008:
1. Open the enrollWithIX509EnrollmentHelper.sln with Visual Studio 2008.
2. Click on the "Build Solution" button on toolbar.

Build with Windows SDK CMD Shell:
1. Open the Windows SDK CMD Shell by clicking Start -> All Programs -> Microsoft Windows SDK v7.0 -> CMD Shell.
2. Run the command line "msbuild.exe enrollWithIX509EnrollmentHelper.sln".

Steps to add COM reference to CertEnroll.dll in your own project:
In order to use CertEnroll, you need to add COM Reference from 
\windows\system32\certenroll.dll, visual studio will automatically generate 
Interop.certenrollLib.dll for you. Below are the detail steps:

1. click 'Project' tab
2. click 'Add Reference...'
3. click 'COM' tab
4. select 'CertEnroll 1.0 Type Library	1.0	%SystemDrive%\windows\system32\CertEnroll.dll"
5. click 'OK'

Then in your c# codes, you only need add 
'using certenrollLib'

Usage:

enrollWithIX509EnrollmentHelper.exe <-Param> <Value>

-Param                       Value
-Context                     User | Machine
-TemplateName                Certificate template name
-PolicyServerAuthType        Kerberos | UsernamePassword | Certificate
-PolicyServerUrl             Policy server URL
-PolicyServerUsername        Username or auth cert hash for policy server authentication
-PolicyServerPassword        Password for policy server authentication
-EnrollmentServerAuthType    Kerberos | UsernamePassword | Certificate
-EnrollmentServerUrl         Enrollment server URL
-EnrollmentServerUsername    Username or auth cert hash for enrollment server authentication
-EnrollmentServerPassword    Password for enrollment server authentication

Example:
enrollWithIX509EnrollmentHelper.exe -Context User -TemplateName User -PolicyServerAuthType UsernamePassword -P
olicyServerUrl https://policyservermachinename.sampledomain.sample.com/ADPolicyProvider_CEP_UsernamePassword/s
ervice.svc/CEP -PolicyServerUsername sampledomain\sampleuser -PolicyServerPassword samplepassword -EnrollmentS
erverAuthType UsernamePassword -EnrollmentServerUrl https://enrollmentservermachinename.sampledomain.sample.co
m/CaName_CES_UsernamePassword/service.svc/CES -EnrollmentServerUsername sampledomain\samleuser -EnrollmentServ
erPassword samplepassword