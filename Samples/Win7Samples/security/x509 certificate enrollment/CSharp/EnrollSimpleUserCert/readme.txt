Readme - Using CertEnroll to Enroll For a User Certificate From Template

DESCRIPTION:
The sample shows how to enroll for a user certificate based on the given
template, set subject name and friendly name.

If the template requires that the request provide with the subject name, then
the given subject name will be used, otherwise, if the template requires that
the subject name be set based on information in AD, then the given subject
name will be ignored by CA. The friendly name should always be set on the 
certificate.

PLATFORM:
The runtimes requires Windows Vista, it only works on a domain joined machine.
 
FILES:
Program.cs
EnrollCertificate.cs
EnrollSimpleUserCert.sln
EnrollSimpleUserCert.csproj
Properties\AssemblyInfo.cs

BUILD INSTRUCTIONS:
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
    EnrollSimpleUserCert <Template> <SubjectName> <FriendlyName>
    Example: EnrollSimpleUserCert User cn=MyCert "My Cert"
             EnrollSimpleUserCert User "cn=MyCert,OU=myOu" MyCert

KEY APIS USED IN THE SAMPLE:
CX509Enrollment:InitializeFromTemplateName()
IX509CertificateRequest:GetInnerRequest()
CX500DistinguishedName.Encode
CX509Enrollment:Enroll()