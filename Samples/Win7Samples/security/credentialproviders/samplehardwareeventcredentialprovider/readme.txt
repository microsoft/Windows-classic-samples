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
This sample implements a hardware event credential provider. In this scenario, the hardware
event (connect/disconnect) is emulated by a window button toggle. 

How to run this sample
--------------------------------
Once you have built the project, copy SampleHardwareEventCredentialProvider.dll to 
the System32 directory and run register.reg from an elevated command prompt. The 
credential should appear the next time a logon is invoked (such as when switching users).

What this sample demonstrates
-----------------------------
This sample demonstrates how to handle asynchronous events by updating UI shown in logonUI for your credential provider.

