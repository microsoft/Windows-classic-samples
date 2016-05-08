'---------------------------------------------------------------------
'  This file is part of the Microsoft .NET Framework SDK Code Samples.
' 
'  Copyright (C) Microsoft Corporation.  All rights reserved.
' 
'This source code is intended only as a supplement to Microsoft
'Development Tools and/or on-line documentation.  See these other
'materials for detailed information regarding Microsoft code samples.
' 
'THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
'KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
'IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
'PARTICULAR PURPOSE.
'---------------------------------------------------------------------


' Simple template based user enrollment.
' Add Subject name and set key size.

Option Explicit

' Constants for enroll context
Const ContextUser                       = 1
Const ContextMachine                    = 2
Const ContextAdministratorForceMachine  = 3

' Constants for enrollment status
Const Enrolled              = 1
Const EnrollPended          = 2

' Constants for GetInnerRequest parameter
Const LevelInnerMost        = 0
Const LevelNext             = 1

' Constants for certificate name string type
Const XCN_CERT_NAME_STR_NONE            = 0

Dim TemplateName
Dim SubjectName
Dim KeyLength
Dim X509Enrollment
Dim Pkcs10Request
Dim PrivateKey
Dim X500DistinguishedName
Dim X509EnrollmentStatus
Dim EnrollmentStatus

' Parse the command line
If WScript.Arguments.Count = 3 Then
    TemplateName = WScript.Arguments.Item(0)
    SubjectName = WScript.Arguments.Item(1)
    KeyLength = WScript.Arguments.Item(2)
Else
    PrintUsage
End If

' Create IX509CertificateRequestPkcs10
Set Pkcs10Request = CreateObject("X509Enrollment.CX509CertificateRequestPkcs10")
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Initialize IX509CertificateRequestPkcs10
Call Pkcs10Request.InitializeFromTemplateName(ContextUser, TemplateName)
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Retrieve the private key
Set PrivateKey = Pkcs10Request.PrivateKey
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Set the key length
PrivateKey.Length = KeyLength
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Create IX500DistinguishedName
Set X500DistinguishedName = CreateObject("X509Enrollment.CX500DistinguishedName")
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Encode the subject name
Call X500DistinguishedName.Encode(SubjectName, XCN_CERT_NAME_STR_NONE)
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Add subject name into the request
Pkcs10Request.Subject = X500DistinguishedName  
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Create IX509Enrollment
Set X509Enrollment = CreateObject("X509Enrollment.CX509Enrollment")
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Initialize IX509Enrollment
Call X509Enrollment.InitializeFromRequest(Pkcs10Request)
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Enroll
Call X509Enrollment.Enroll()
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Get IX509EnrollmentStatus
Set X509EnrollmentStatus = X509Enrollment.Status
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Get enrollment status
EnrollmentStatus = X509EnrollmentStatus.Status
If (Err.Number <> 0) Then
    Wscript.Echo Err.Number & Err.Description
    Wscript.Quit(-1)
End If

' Display result
If (EnrollmentStatus = Enrolled) Then
    Wscript.Echo "Cert Issued: " & X509EnrollmentStatus.ErrorText & "--" & X509EnrollmentStatus.Text
Elseif (EnrollmentStatus = EnrollPended) Then
    Wscript.Echo "Request is pending: " & X509EnrollmentStatus.ErrorText & "--" & X509EnrollmentStatus.Text
Else
    Wscript.Echo "Request Failed: " & X509EnrollmentStatus.ErrorText & "--" & X509EnrollmentStatus.Text
End If

Wscript.Quit(0)

' PrintUsage function
Sub PrintUsage
    Wscript.Echo "Usage: enrollSimpleUserCert.vbs <Template> <SubjectName> <KeyLength>"
    Wscript.Echo "Example: enrollSimpleUserCert.vbs User ""CN=foo,OU=bar,DC=com"" 1024"
    Wscript.Quit(-1)
End Sub ' PrintUsage



'' SIG '' Begin signature block
'' SIG '' MIIQggYJKoZIhvcNAQcCoIIQczCCEG8CAQExCzAJBgUr
'' SIG '' DgMCGgUAMGcGCisGAQQBgjcCAQSgWTBXMDIGCisGAQQB
'' SIG '' gjcCAR4wJAIBAQQQTvApFpkntU2P5azhDxfrqwIBAAIB
'' SIG '' AAIBAAIBAAIBADAhMAkGBSsOAwIaBQAEFK9jbCW1ECZs
'' SIG '' UYefpNzhSlBjkwQ8oIIORTCCBBMwggNAoAMCAQICEGoL
'' SIG '' mU/AACKrEdsCQnwC074wCQYFKw4DAh0FADB1MSswKQYD
'' SIG '' VQQLEyJDb3B5cmlnaHQgKGMpIDE5OTkgTWljcm9zb2Z0
'' SIG '' IENvcnAuMR4wHAYDVQQLExVNaWNyb3NvZnQgQ29ycG9y
'' SIG '' YXRpb24xJjAkBgNVBAMTHU1pY3Jvc29mdCBUZXN0IFJv
'' SIG '' b3QgQXV0aG9yaXR5MB4XDTA2MDYyMjIyNTczMVoXDTEx
'' SIG '' MDYyMTA3MDAwMFowcTELMAkGA1UEBhMCVVMxEzARBgNV
'' SIG '' BAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1vbmQx
'' SIG '' HjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEb
'' SIG '' MBkGA1UEAxMSTWljcm9zb2Z0IFRlc3QgUENBMIIBIjAN
'' SIG '' BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAj/Pz33qn
'' SIG '' cihhfpDzgWdPPEKAs8NyTe9/EGW4StfGTaxnm6+j/cTt
'' SIG '' fDRsVXNecQkcoKI69WVT1NzP8zOjWjMsV81IIbelJDAx
'' SIG '' UzWp2tnbdH9MLnhnzdvJ7bGPt67/eW+sIwZUDiNDN3jd
'' SIG '' Pk4KbdAq9sZ+W5J0DbMTD1yxcbQQ/LEgCAgueW5f0nI0
'' SIG '' rpI6gbAyrM5DWTCmwfyu+MzofYZrXK7r3pX6Kjl1BlxB
'' SIG '' OlHcVzVOksssnXuk3Jrp/iGcYR87pEx/UrGFOWR9kYlv
'' SIG '' nhRCs7yi2moXhyTmG9V8fY+q3ALJoV7d/YEqnybDNkHT
'' SIG '' z/xzDRx0KDjypQrF0Q+7077QkwIDAQABo4HrMIHoMIGo
'' SIG '' BgNVHQEEgaAwgZ2AEMBjRdejAX15xXp6XyjbQ9ahdzB1
'' SIG '' MSswKQYDVQQLEyJDb3B5cmlnaHQgKGMpIDE5OTkgTWlj
'' SIG '' cm9zb2Z0IENvcnAuMR4wHAYDVQQLExVNaWNyb3NvZnQg
'' SIG '' Q29ycG9yYXRpb24xJjAkBgNVBAMTHU1pY3Jvc29mdCBU
'' SIG '' ZXN0IFJvb3QgQXV0aG9yaXR5ghBf6k/S8h1DELboVD7Y
'' SIG '' lSYYMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFFSl
'' SIG '' IUygrm+cYE4Pzt1G1ddh1hesMAsGA1UdDwQEAwIBhjAJ
'' SIG '' BgUrDgMCHQUAA4HBACzODwWw7h9lGeKjJ7yc936jJard
'' SIG '' LMfrxQKBMZfJTb9MWDDIJ9WniM6epQ7vmTWM9Q4cLMy2
'' SIG '' kMGgdc3mffQLETF6g/v+aEzFG5tUqingK125JFP57MGc
'' SIG '' JYMlQGO3KUIcedPC8cyj+oYwi6tbSpDLRCCQ7MAFS15r
'' SIG '' 4Dnxn783pZ5nSXh1o+NrSz5mbGusDIj0ujHBCqblI96+
'' SIG '' Rk7oVQ2DI3oQkSmGQf+BrmRXoJfB3YuXXFc+F88beLHS
'' SIG '' F0S8oJhPjzCCBM8wggO3oAMCAQICCmECiAAAAAAAAA8w
'' SIG '' DQYJKoZIhvcNAQEFBQAwcTELMAkGA1UEBhMCVVMxEzAR
'' SIG '' BgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1v
'' SIG '' bmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlv
'' SIG '' bjEbMBkGA1UEAxMSTWljcm9zb2Z0IFRlc3QgUENBMB4X
'' SIG '' DTA5MDMwOTIzMjgwNloXDTEwMDkwOTIzMzgwNlowgYEx
'' SIG '' EzARBgoJkiaJk/IsZAEZFgNjb20xGTAXBgoJkiaJk/Is
'' SIG '' ZAEZFgltaWNyb3NvZnQxFDASBgoJkiaJk/IsZAEZFgRj
'' SIG '' b3JwMRcwFQYKCZImiZPyLGQBGRYHcmVkbW9uZDEgMB4G
'' SIG '' A1UEAxMXTVNJVCBUZXN0IENvZGVTaWduIENBIDIwggEi
'' SIG '' MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC1F4Jo
'' SIG '' THUr7NcGiknSPnOVQeHc0+UGNEV/tpaprkcxdj0Zn6Z3
'' SIG '' jyaMgvrJ0kR36RDbGc4TLe0G3Yfy0M0dyfPLXcBeaJnS
'' SIG '' q4maGoiw/KQ/drxlr/cYjyqS8Db9SlrYlMHR6ZF9WdTK
'' SIG '' 13rZ3ENj1CAj6XsHIv71U3AMcr6awZVTGsY6cACvBUy5
'' SIG '' pmoo60qhnIF4J5obN6U8W6ofrtxDnSu2sFCUpPn0gsJR
'' SIG '' PSRGRDs+t9ERv5nFB3GaKvQB2u+fdIsy4TlXQUeg5NUp
'' SIG '' n+FkugjpvLiA09RtY8uSs0zYOVJznwa/Sbw+FyykVqKO
'' SIG '' 9+d0aAc8MvX+zaeJW1HEol33cy7XAgMBAAGjggFWMIIB
'' SIG '' UjASBgkrBgEEAYI3FQEEBQIDAQABMCMGCSsGAQQBgjcV
'' SIG '' AgQWBBRkEWKEELgKtHrLYOT7ZRJijPOfhzAdBgNVHQ4E
'' SIG '' FgQUzDyyQ7tWnJSHCOXDB6Ye2Clw9SYwGQYJKwYBBAGC
'' SIG '' NxQCBAweCgBTAHUAYgBDAEEwCwYDVR0PBAQDAgGGMA8G
'' SIG '' A1UdEwEB/wQFMAMBAf8wHwYDVR0jBBgwFoAUVKUhTKCu
'' SIG '' b5xgTg/O3UbV12HWF6wwTAYDVR0fBEUwQzBBoD+gPYY7
'' SIG '' aHR0cDovL2NybC5taWNyb3NvZnQuY29tL3BraS9jcmwv
'' SIG '' cHJvZHVjdHMvbGVnYWN5dGVzdHBjYS5jcmwwUAYIKwYB
'' SIG '' BQUHAQEERDBCMEAGCCsGAQUFBzAChjRodHRwOi8vd3d3
'' SIG '' Lm1pY3Jvc29mdC5jb20vcGtpL2NlcnRzL0xlZ2FjeVRl
'' SIG '' c3RQQ0EuY3J0MA0GCSqGSIb3DQEBBQUAA4IBAQAasHDL
'' SIG '' oaKaboHsHhwfzragkNcF0NcW8zVVwhpnAPjFG6p/TRT5
'' SIG '' dc1+JdEnTYDpYu9oIKu0ak4C8WVYByeK3YrJkQWEyLsp
'' SIG '' bXOSLCHP0y4HdAMcXS3fQNpjGKXWXwe1fJAITTJMxh9+
'' SIG '' PI44vHLxucYSrk/Ai7zkjbeU7DsZZYJgtCSgk0D9aZNy
'' SIG '' PgQqwWyIT+GhuES7uyHOuWNdGDAPNXtExXZQqAC/IYhE
'' SIG '' l0JF4Q81SP7BcKsxY2f+ZeJrW/6LaTbrGp9pXRs2YHLU
'' SIG '' dKWEDFUlLFdpHNPGezG1ZPUYOfOLHKcCKh2N8hjfQfQD
'' SIG '' 9d+FxwR4U96dwqY+Fqpg8JyV3vwlMIIFVzCCBD+gAwIB
'' SIG '' AgIKNPS8QgABAACwYzANBgkqhkiG9w0BAQUFADCBgTET
'' SIG '' MBEGCgmSJomT8ixkARkWA2NvbTEZMBcGCgmSJomT8ixk
'' SIG '' ARkWCW1pY3Jvc29mdDEUMBIGCgmSJomT8ixkARkWBGNv
'' SIG '' cnAxFzAVBgoJkiaJk/IsZAEZFgdyZWRtb25kMSAwHgYD
'' SIG '' VQQDExdNU0lUIFRlc3QgQ29kZVNpZ24gQ0EgMjAeFw0w
'' SIG '' OTAzMzEwMTQ4MzZaFw0xMDAzMzEwMTQ4MzZaMBUxEzAR
'' SIG '' BgNVBAMTClZTIEJsZCBMYWIwgZ8wDQYJKoZIhvcNAQEB
'' SIG '' BQADgY0AMIGJAoGBAL6hqBKHP1U+um2LEEKprMKMYxau
'' SIG '' 4Dgu/TePMSAURHNcre7kuTaLapQBvVgRbmpqjcwhXrQV
'' SIG '' /Imvzqur0PR9/hiavuSV84v0v39rpuN9dKxfXpWNVkp0
'' SIG '' 15zhzn7/Wl6a6BimrklH6u+z/YnCAZMkZtfmoN4vq5h/
'' SIG '' cLnyn+YUshZ9AgMBAAGjggK+MIICujALBgNVHQ8EBAMC
'' SIG '' B4AwHQYDVR0OBBYEFJvFUIz41jpDftUwfBKOauKb4B3z
'' SIG '' MD0GCSsGAQQBgjcVBwQwMC4GJisGAQQBgjcVCIPPiU2t
'' SIG '' 8gKFoZ8MgvrKfYHh+3SBT4KusGqH9P0yAgFkAgELMB8G
'' SIG '' A1UdIwQYMBaAFMw8skO7VpyUhwjlwwemHtgpcPUmMIHx
'' SIG '' BgNVHR8EgekwgeYwgeOggeCggd2GOWh0dHA6Ly9jb3Jw
'' SIG '' cGtpL2NybC9NU0lUJTIwVGVzdCUyMENvZGVTaWduJTIw
'' SIG '' Q0ElMjAyKDEpLmNybIZQaHR0cDovL21zY3JsLm1pY3Jv
'' SIG '' c29mdC5jb20vcGtpL21zY29ycC9jcmwvTVNJVCUyMFRl
'' SIG '' c3QlMjBDb2RlU2lnbiUyMENBJTIwMigxKS5jcmyGTmh0
'' SIG '' dHA6Ly9jcmwubWljcm9zb2Z0LmNvbS9wa2kvbXNjb3Jw
'' SIG '' L2NybC9NU0lUJTIwVGVzdCUyMENvZGVTaWduJTIwQ0El
'' SIG '' MjAyKDEpLmNybDCBrwYIKwYBBQUHAQEEgaIwgZ8wRQYI
'' SIG '' KwYBBQUHMAKGOWh0dHA6Ly9jb3JwcGtpL2FpYS9NU0lU
'' SIG '' JTIwVGVzdCUyMENvZGVTaWduJTIwQ0ElMjAyKDEpLmNy
'' SIG '' dDBWBggrBgEFBQcwAoZKaHR0cDovL3d3dy5taWNyb3Nv
'' SIG '' ZnQuY29tL3BraS9tc2NvcnAvTVNJVCUyMFRlc3QlMjBD
'' SIG '' b2RlU2lnbiUyMENBJTIwMigxKS5jcnQwHwYDVR0lBBgw
'' SIG '' FgYKKwYBBAGCNwoDBgYIKwYBBQUHAwMwKQYJKwYBBAGC
'' SIG '' NxUKBBwwGjAMBgorBgEEAYI3CgMGMAoGCCsGAQUFBwMD
'' SIG '' MDoGA1UdEQQzMDGgLwYKKwYBBAGCNxQCA6AhDB9kbGFi
'' SIG '' QHJlZG1vbmQuY29ycC5taWNyb3NvZnQuY29tMA0GCSqG
'' SIG '' SIb3DQEBBQUAA4IBAQCL06G31/BXGahMYqojp0hID3q/
'' SIG '' daaznfhEWrlT/7GmY9nGmjO72+aFQuSSvex3i0KnyrEO
'' SIG '' Fj1HEIDu28Su1IjvzZZuqLWvZu5fZalbo6lRP7JZZnqP
'' SIG '' 06q8uQuBJyYG8/GH/mBAMJznN934gOqKXItMOuLbr9lX
'' SIG '' ZVaRKc5o+i+YpEFlOPF0Od5lBLe+ik5fWBOU3VcHJ9k+
'' SIG '' yUOXt+Ow/rkShGpQv0hSSN0Jcmt06zWJcnrMF6TG27Nf
'' SIG '' VUQuoyuZm6XvoxXQhfpehTjL9d9q81fO9njLBIV/C3HG
'' SIG '' 8zWdSxTmciaEJLTfC+UfVPrxwixtvSDHsSZ3nWDH4rXS
'' SIG '' WLWCacvwMYIBqTCCAaUCAQEwgZAwgYExEzARBgoJkiaJ
'' SIG '' k/IsZAEZFgNjb20xGTAXBgoJkiaJk/IsZAEZFgltaWNy
'' SIG '' b3NvZnQxFDASBgoJkiaJk/IsZAEZFgRjb3JwMRcwFQYK
'' SIG '' CZImiZPyLGQBGRYHcmVkbW9uZDEgMB4GA1UEAxMXTVNJ
'' SIG '' VCBUZXN0IENvZGVTaWduIENBIDICCjT0vEIAAQAAsGMw
'' SIG '' CQYFKw4DAhoFAKBwMBAGCisGAQQBgjcCAQwxAjAAMBkG
'' SIG '' CSqGSIb3DQEJAzEMBgorBgEEAYI3AgEEMBwGCisGAQQB
'' SIG '' gjcCAQsxDjAMBgorBgEEAYI3AgEVMCMGCSqGSIb3DQEJ
'' SIG '' BDEWBBTvbCubAiqCTbk8N1i7lYSMOtXJ6zANBgkqhkiG
'' SIG '' 9w0BAQEFAASBgGyHD4HjhpdnudHeccoJB5QiMk7GPAfJ
'' SIG '' yfGDb6ox/QpPDNd3/EHuVGi77Dz5svZ3yvkelvLdj4od
'' SIG '' xRucGOaLNt3N537SGQFJYN9AiWzU5oOvHv9kqgUyMJlk
'' SIG '' 84U0CxrZN1o6eL7Z8L6QhNUA+aVGoO67tEeYxS2jhblE
'' SIG '' 3rhLe8AW
'' SIG '' End signature block
