Windows Vista Certenroll VBScript Sample

Description:
This sample demonstrates how to create a simple template based user request
and enroll using certenroll API.

Files:
enrollSimpleUserCert.vbs            VBScript file
Readme.txt                          This file

Platform
This sample requires Windows Vista.

How to run:
1. Open command window and navigate to the directory
2. Use cscript to execute the vbscript file

Usage:
cscript enrollSimpleUserCert.vbs <Template> <SubjectName> <KeyLength>

<Template>  
User template name to enroll

<SubjectName>
Subject name

<KeyLength>
Private key length

Example: 
cscript enrollSimpleUserCert.vbs User "CN=foo,OU=bar,DC=com" 1024
