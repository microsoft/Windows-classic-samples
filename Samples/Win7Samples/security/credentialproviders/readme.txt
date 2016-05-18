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
This sample demonstrates how to use the credential provider APIs and interfaces such as ICredentialProvider, ICredentialProviderCredential, and ICredentialProviderFilter.

How to build these samples
--------------------------------
The top-level directory contains a solution file, CredentialProviderSamples.sln, that can be opened in Visual Studio or built using msbuild.exe in the SDK command line environment.

How to run this sample
--------------------------------
Each subfolder (for example, SampleCredentialProvider\ or SampleCredentialFilter\) contains a readme.txt file discussing how to run that sample.

Contents of this sample's subfolders:
-------------------
Helpers: common files for the other subfolders.

SampleAllControlsCredentialProvider: demonstrates the use of all credential provider field types.

SampleCredentialFilter: demonstrates the use of ICredentialProviderFilter.

SampleCredentialProvider: demonstrates the basic use of ICredentialProvider.

SampleCredUICredentialProvider: demonstrates a credential provider that runs in the CredUIPromptForWindowsCredentials dialog.

SampleHardwareEventCredentialProvider: demonstrates how a credential provider can respond to a hardware event such as smart card removal/insertion.

SampleWrapExistingCredentialProvider: demonstrates how a credential provider can "wrap" or contain another credential provider in order to add functionality.