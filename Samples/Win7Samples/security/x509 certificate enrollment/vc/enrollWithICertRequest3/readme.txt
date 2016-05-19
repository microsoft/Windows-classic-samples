Windows 7 X509CertificateEnrollment C++ Sample

Sample name: enrollWithICertRequest3

Description:
This sample demonstrates how to use the Windows 7 new http protocol to 
enroll a certificate by calling the IX509Enrollment2:CreateRequest, 
ICertRequest3::SetCredential, ICertRequest3::Submit and 
IX509Enrollment2::InstallResponse2 methods. The purpose of the call to
the ICertRequest3::SetCredential is to set the authentication credential
to enrollment server in the object pointed by the interface ICertRequest3.

Files:
enrollWithICertRequest3.cpp		C++ source file
enrollWithICertRequest3.sln		Solution file
enrollWithICertRequest3.vcproj		Project file
readme.txt				This file

Platform:
This sample requires Windows 7.

Build with Visual Studio 2008:
1. Open the enrollWithICertRequest3.sln with Visual Studio 2008.
2. Click on the "Build Solution" button on toolbar.

Build with Windows SDK CMD Shell:
1. Open the Windows SDK CMD Shell by clicking Start -> All Programs -> Microsoft Windows SDK v7.0 -> CMD Shell.
2. Run the command line "msbuild.exe enrollWithICertRequest3.sln".

Usage:
enrollWithICertRequest3.exe <-Param> <Value>

-Param                       Value
-Context                     User | Machine
-TemplateName                Certificate template name
-PolicyServerAuthType        Kerberos | UsernamePassword | Certificate
-PolicyServerUrl             Policy server URL
-PolicyServerUsername        Username or auth cert hash for policy server authentication
-PolicyServerPassword        Password for policyserver authentication
-EnrollmentServerAuthType    Kerberos | UsernamePassword | Certificate
-EnrollmentServerUrl         Enrollment server URL
-EnrollmentServerUsername    Username or auth cert hash for enrollment server authentication
-EnrollmentServerPassword    Password for enrollment server authentication

Example:
enrollWithICertRequest3.exe -Context User -TemplateName User -PolicyServerAuthType Certificate -PolicyServerUr
l https://policyservermachinename.sampledomain.sample.com/ADPolicyProvider_CEP_Certificate/service.svc/CEP -Po
licyServerUsername 02aea105e66a8a2d41a7f630517db0d2c0de625b -EnrollmentServerAuthType UsernamePassword -Enroll
mentServerUrl https://enrollmentservermachinename.sampledomain.sample.com/CaName_CES_UsernamePassword/service.
svc/CES -EnrollmentServerUsername sampledomain\sampleuser -EnrollmentServerPassword samplepassword