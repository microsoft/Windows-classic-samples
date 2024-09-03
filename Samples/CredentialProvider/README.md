---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: V2 Credential Provider Sample
urlFragment: credential-provider
description: Demonstrates how to build a v2 credential provider that makes use of the new capabilities in credential provider framework in Windows 8 and Windows 8.1.
---

# V2 Credential Provider Sample

This sample implements a simple v2 credential provider. A credential provider allows a 3rd 
party to provide an alternate way of logging on. For example, a fingerprint solution vendor 
would write a credential provider to interact with the user and send the appropriate 
credentials to the system for authentication.

This sample implements a simplified credential provider that is based on the password 
credential provider that ships with Windows.  When run, the credential provider 
should enumerate two tiles, one for the administrator and one for the guest account. Note 
that the guest account must be enabled for that tile to log on successfully. (The guest account
is disabled by default.)

This sample demonstrates simple password based log on and unlock behavior.  It also shows how to construct
a simple user tile and handle the user interaction with that tile.

Note that this sample does not demonstrate the following:
- other credential provider scenarios, like participating in credui or handling change password.  
- every possible field that you can put in your user tile
- any network access prior to local authentication (the Pre-Logon Access Provider (PLAP) behavior)
- an event based credential provider (like a smartcard or fingerprint based system)

## Operating system requirements

Client

Windows 8.1

Server

Windows Server 2012 R2

## Build the sample

1. Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2. Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3. Press F7 or use **Build** \> **Build Solution** to build the sample.

## Run the sample

Copy SampleV2CredentialProvider.dll to the System32 directory
and run Register.reg from an elevated command prompt. The credential should appear the next
time a logon is invoked (such as when logging off or rebooting the machine).

