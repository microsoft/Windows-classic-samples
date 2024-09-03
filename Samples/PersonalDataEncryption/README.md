---
page_type: sample
languages:
- cpp
- cppwinrt
products:
- windows-api-win32
name: Personal Data Encryption sample (Win32)
urlFragment: PersonalDataEncryption
description: "Demonstrates the usage of Personal Data Encryption (Windows Data Protection)."
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Personal Data Encryption sample

Shows how to use [Personal Data Encryption](https://learn.microsoft.com/windows/security/operating-system-security/data-protection/personal-data-encryption/)
for protecting user files and memory buffers.

Personal Data Encryption can protect data either while a user is signed out ("Level 1" or "L1") or while a system is locked ("Level 2" or "L2").

The sample shows the following:

- Marking a folder so that all new files are protected at a particular level, or removing protection.
- Protecting a file at a particular level, or removing protection.
- Protecting a memory buffer at a particular level, or removing protection.

**Note** This sample requires Visual Studio to build and Windows 10 to execute.
 
To obtain information about Windows development, go to the [Windows Dev Center](http://go.microsoft.com/fwlink/?LinkID=532421)

To obtain information about Microsoft Visual Studio and the tools for developing Windows apps, go to [Visual Studio](http://go.microsoft.com/fwlink/?LinkID=532422)

## Related topics

### Reference

[UserDataProtectionManager class](https://learn.microsoft.com/uwp/api/windows.security.dataprotection.userdataprotectionmanager)

### Related samples

* [Personal Data Encryption UWP sample](https://github.com/microsoft/Windows-universal-samples/tree/main/Samples/PersonalDataEncryption)

## System requirements

* Windows 10 Version 1903 (build 18362) or higher

## Build the sample

To build this sample:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

## Run the sample

The next steps depend on whether you just want to deploy the sample or you want to both deploy and run it.

### Deploying the sample

- Select Build \> Deploy Solution. 

### Deploying and running the sample

- Set the Debugging options to specify the desired command line options.
- To debug the sample and then run it, press F5 or select Debug \> Start Debugging. To run the sample without debugging, press Ctrl+F5 or select Debug \> Start Without Debugging. 
