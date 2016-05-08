Windows Vista Certenroll VBScript Sample

Description:
This sample demonstrates how to create a simple template based machine request
and enroll using certenroll API.

Files:
enrollSimpleMachineCert.vbs         VBScript file
Readme.txt                          This file

Platform
This sample requires Windows Vista.

How to run:
1. Open command window and navigate to the directory
2. Use cscript to execute the vbscript file

Usage:
cscript enrollSimpleMachineCert.vbs <Template> <CertFriendlyName> <CertDescription>

<Template>  
Machine template name to enroll

<CertFriendlyName>
Certificate friendly name

<CertDescription>
Certificate description

Example: 
cscript enrollSimpleMachineCert.vbs Machine "Machine Cert" "Simple Machine Cert"