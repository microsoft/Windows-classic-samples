---
page_type: sample
languages:
- c#
products:
- windows-api-win32
name: PDE API Sample Application
urlFragment: PDESampleApplication
description: Demonstrates the usage of the Personal Data Encryption API (Windows DataProtection API)
extendedZipContent:
- path: LICENSE
  target: LICENSE
---


PDE API Sample Application
============================
Please refer to this page (https://learn.microsoft.com/en-us/windows/security/operating-system-security/data-protection/personal-data-encryption/) for a high level overview of PDE and its capabilities

This sample demonstrates how to use the Personal Data Encryption API (https://learn.microsoft.com/en-us/uwp/api/windows.security.dataprotection?view=winrt-22621) to protect/unprotect Folders, Files and Buffers to the two different levels of security. The sample demonstrates how to perform each of the following operations:

-   Protect a folder and all its contents to L1/L2 level of security.
-   Unprotect all the contents of a folder.
-   Protect a file to L1/L2 level of security.
-   Unprotect a file.
-   Protect buffer text with L1/L2 level of security.
-   Unprotect buffer text to its original 

Operating system requirements
-----------------------------

Client: Windows 11 22H2 and above

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled PDESampleApplication.sln.

3.  Use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

Goto the bin/Release folder and click on PDETestApp.exe. Browse to the folder/file to protect and click on the Protect L1/L2 buttons to protect the content and Unprotect to remove any protection on them. Similarly enter the text to protect in the buffer and put in the protected text into the input text box to unprotect.
