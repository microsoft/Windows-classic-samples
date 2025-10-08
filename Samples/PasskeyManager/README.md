---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Passkey Manager sample
urlFragment: passkeymanager
description: Demonstrates how apps can implement an integrated plugin passkey manager.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Passkey Manager sample

This sample demonstrates how to integrate a third-party passkey manager with the Windows platform using WebAuthn Plugin APIs.

* Registering the passkey manager app as a COM object in the system registry.
* Implementing the `IPluginAuthenticator` interface for managing webauthn credentials.
* Using the `WebAuthNPluginAddAuthenticator` API to add the plugin authenticator.
* Handling credential creation requests from webauthn clients.
* Managing credential metadata with `WebAuthNPluginAuthenticatorAddCredentials` and `WebAuthNPluginAuthenticatorRemoveCredentials`.
* Performing user verification using Windows Hello with `WebAuthNPluginPerformUv`.
* Updating plugin authenticator details with `WebAuthNPluginUpdateAuthenticatorDetails`.
* Removing the plugin authenticator with `WebAuthNPluginRemoveAuthenticator`.
* Responding to system-initiated operations like `PluginCancelOperation`.

**Note**   The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server.

This sample was created for Windows 11 Insider Edition Beta Channel 10.0.22635.4515 using Visual Studio and the Windows SDK 10.0.26100.1742. 

**Note**: `These are experimental APIs and are not guaranteed to be included in future versions of Windows`.

## Operating system requirements

Windows 11 Insider Edition Beta Channel. Build Major Version = 22635 and Minor Version >= 4515.

#### Windows Insider Program
* To sign up for the Windows Insider Program (WIP) you must register here: [Windows Insider Program](https://www.microsoft.com/en-us/windowsinsider/register).
* Once you have signed up for WIP, go to *Settings > Windows Update > WIP* and select Beta channel. Video instructions: https://www.microsoft.com/en-us/videoplayer/embed/RW1kR8G?download=false


## Build the sample

#### Download Prerequisites

* Download the [cbor-lite](https://bitbucket.org/isode/cbor-lite/src/master/include/) directory and copy it into the `include` directory of the sample.
* Download the [webauthn](https://github.com/microsoft/webauthn) directory and copy it into the `include` directory of the sample.

#### Fill in the plugin information.
* Use guidgen.exe to generate a new GUID for the plugin.
* Open `Package.appxmanifest` and replace the `Clsid` with the new GUID. Also, replace the `DisplayName` and `PublisherDisplayName` with your plugin's name and your organization's name.
* Open the `PluginManagement/PluginRegistrationManager.h` file and fill in the plugin's `Name`, `AAGUID`, `RpId`, and `Clsid` fields with your plugin information.

#### Build
* Install the Windows SDK version 10.0.26100.1742 or higher.
* Open the solution (*.sln*) file titled *PasskeyManager.sln* from Visual Studio.
* Press Ctrl+Shift+B or select **Build** \> **Build Solution**.

## Run the sample

Press F5 or select Debug \> Start Debugging. To run the sample without debugging, press Ctrl+F5 or select Debug \> Start Without Debugging. 

Things to try with the sample:

* Use the Register button to register the plugin on the system.

* Go to *Settings > Accounts > Passkeys > Advanced Options* and enable __Contoso Passkey Manager__.

* Create a new passkey.
  * Visit webauthn test websites, like https://webauthn.io, or use test accounts on websites like github.com, linkedin.com, or amazon.com.
  * Press *Continue* when asked to save the passkey to __Contoso Passkey Manager__.
  * Or pick out __Contoso Passkey Manager__ from the list of authenticators"
* Click the *Add Passkeys* button to add your new passkeys to the system cache.
  * Look out for the `Autofill` label next to the username in the Passkeys list.
* On supported browsers like Edge and Chrome visit websites that support autofill for passkeys.
  * Click on the username or password text input fields to see the autofill dropdown show up.
  * Your new passkey should show up in the dropdown.
* You can also delete the passkey from the system cache by clicking on the *Delete Passkeys* button.
* Use *Simulate Vault Unlock* toggle to switch between using the new `WebAuthNPluginPerformUv` and other existing methods to invoke a Windows Hello prompt.
* Use the *Minimize UI* toggle to hide the plugin UI prompt asking confirmation during the make Save passkey and Sign in with passkey flow.


## Files

`PluginAuthenticatorImpl.cpp/.h` These files implement the PluginAuthenticator interface for managing WebAuthn credentials.

`PluginRegistrationManager.cpp/.h` These files handle the registration, unregistration, and state management of the plugin authenticator using WebAuthn Plugin APIs.

`PluginCredentialManager.cpp/.h` These files manage the addition and removal of plugin credentials using WebAuthn Plugin APIs, including handling credential metadata and interfacing with the WebAuthNPluginAuthenticatorAddCredentials API.

`App.xaml.cpp/.h` These files initialize the application, handle the main application lifecycle events, and manage the registration of the plugin as a COM object, including setting up the main window and handling plugin operations.

`GetAssertion.xaml.cpp/.h` These files handle the user interface and logic for the "Get Assertion" operation in the passkey manager. They manage the retrieval and display of credentials, handle user interactions for selecting credentials, and perform the necessary actions to complete or cancel the plugin operation.

`MakeCredentialPage.xaml.cpp/.h` These files manage the user interface and logic for the "Make Credential" operation in the passkey manager. They handle the creation of new credentials, manage user interactions for confirming or canceling the operation, and update the UI based on the success or failure of the credential creation process.

## Related topics

  [**webauthn**](https://learn.microsoft.com/en-us/windows/win32/api/webauthn/)

  [**microsoft/webauthn**](https://github.com/microsoft/webauthn)

  [**cbor-lite**](https://bitbucket.org/isode/cbor-lite/src/master/include/)

  [**webauthn.io**](https://webauthn.io)

  To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

  To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).
