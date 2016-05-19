Copyright (c) Microsoft Corporation. All rights reserved.

Certificate Selection
=====================
This sample shows how to use the new certificate selection API in Windows 7 CertSelectCertificateChains to select certificates. It also shows how to display the selected certificates to the user for selection using CredUIPromptForWindowsCredentials API.

It demonstrates the following scenario:  
Scenario: Selecting a certificate for signing email
Select a certificate that meets the following criteria
  a.    Is from the user MY store
  b.    Has the SMIME EKU 
  c.    Has the Digital Signature KU
  d.    Is not expired
  
NOTE: THIS SAMPLE WILL NOT COMPILE ON WINDOWS VISTA AS THIS USES NEW APIS AVAILABLE IN WINDOWS 7.

Sample Language Implementations
===============================
C++

Files:
=====
CertSelect.sln
CertSelect.vcproj
CertSelect.cpp

Prerequisites
=============
To build this sample, compile and link it with crypt32.lib, cryptui.lib and credui.lib.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type "msbuild CertSelect.sln"

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. To run this sample, use "CertSelect.exe" from the command prompt.
