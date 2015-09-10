Network Access Protection SHA-SHV-QEC sample
============================================

This sample shows how to create a Quarantine Enforcement Client (QEC), System Health Agent (SHA), and System Health Validator (SHV), for the Network Access Protection (NAP) system.

**Warning**  This sample requires Microsoft Visual Studio 2013 or a later version (any SKU) and doesn't compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[About NAP](http://msdn.microsoft.com/en-us/library/windows/desktop/aa369143)

[NAP Reference](http://msdn.microsoft.com/en-us/library/windows/desktop/aa369706)

Related technologies
--------------------

[Microsoft Media Foundation](http://msdn.microsoft.com/en-us/library/windows/desktop/ms694197)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using Visual Studio:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled SdkSamples.sln from Visual Studio.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

To build the sample in a Command Prompt window:

1.  Open a Command Prompt window and navigate to the sample directory.
2.  Enter **msbuild SdkSamples.sln**.

Run the sample
--------------

To run this sample, follow these steps:

1.  Run **Regsvr32 /s** *PathToSdkShv.dll***\\SdkShv.dll** on the NAP server.
2.  Run **SdkSha.exe /?** on the NAP client for further instructions.
3.  Run **SdkQec.exe /?** on the NAP client for further instructions.

To run SdkShaInfo.dll, follow these steps:

1.  Run SdkSha.exe at a command prompt on the NAP client.
2.  In another Command Prompt window on the NAP client, enter **regsvr32 /s SdkShaInfo.dll**.
3.  Run **netsh nap client show state** on the NAP client and note that the parameters displayed for the SDK SHA sample differ from the parameters displayed when SdkShaInfo.dll is not registered. The SHA\\DLL\\Messages.mc file can be edited to change the parameters displayed for the SDK SHA sample.

To install or uninstall the SHV configuration UI, follow these steps:

1.  Run **Regsvr32 /s** *PathToSdkShv.dll***\\SdkShv.dll** on the NAP server.
2.  Run **Sampleshvui.exe /regserver** to install the sample SHV configuration UI on the NAP server.
3.  Run **Sampleshvui.exe /unregserver** to uninstall the sample SHV configuration UI from the NAP server.

To debug the app and then run it from Visual Studio 2013, press F5 or use **Debug** \> **Start Debugging**. To run the app without debugging, press Ctrl+F5 or use **Debug** \> **Start Without Debugging**.

