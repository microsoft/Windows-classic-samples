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
This sample implements a simple credential provider. This provider wraps the built-in
password provider with two extra fields: a small text and a combobox.

Please note that encapsulation (or "wrapping") should be used sparingly.  It is not a
one size fits all replacement for the GINA chaining behavior.  Unlike GINA chaining,
the behavior you add only applies if the user clicks on your credential tile and does
not apply if they click on another credential tile.  Encapsulation is
only done explicitly and should only be done when you know exactly what the behavior
of the wrapped credprov is.  It should be used when you want to extend the credential
information that the wrapped credprov is getting.  If you merely want to do something
extra with the credentials gathered by another credprov, then a network provider is
likely more suited to your needs than a credential provider.  

How to run this sample
--------------------------------
Once you have built the project, copy SampleWrapExistingCredentialProvider.dll to the
System32 directory and run Register.reg from an elevated command prompt.
The credential should appear the next time a logon is invoked (such as when switching users).

What this sample demonstrates
-----------------------------
This sample demonstrates wrapping another provider and appending extra fields.

