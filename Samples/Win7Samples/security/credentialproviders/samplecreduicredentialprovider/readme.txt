//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

Overview
--------
This sample implements a simple credential provider that works with both LogonUI and CredUI.
A credential provider allows a 3rd party to provide an alternate way of logging on. For 
example, a fingerprint solution vendor would write a credential provider to interact with 
the user and send the appropriate credentials to the system for authentication. More 
information can be found in the Credential Provider document provided in this SDK. 
Questions should be sent to credprov@microsoft.com.

This sample implements a simplified credential provider that is based on the password 
credential provider that ships with Windows.  When run, the credential provider 
should enumerate two tiles, one for the administrator and one for the guest account.  Note 
that if your machine is domain joined, it is likely that the guest account is disabled and 
you will receive an error if you try to use that tile to log on. 

How to run this sample
--------------------------------
Once you have built the project, copy SampleCredUICredentialProvider.dll to the System32
directory and run Register.reg from an elevated command prompt. The credential should appear 
the next time a logon is invoked (such as when switching users).

What this sample demonstrates
-----------------------------
This sample demonstrates simple password based log on and unlock behavior.  It also shows how to construct
a simple user tile and handle the user interaction with that tile.


