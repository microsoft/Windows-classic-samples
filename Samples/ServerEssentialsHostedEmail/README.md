Windows Server Essentials HostedEmail add-in samples
====================================================

This sample describes the key components for a hosted email add-in, a mockup service, and a logging helper for Windows Server Essentials.

Windows Server Essentials allows email service hosts to develop an add-in to integrate Essentials features via the Hosted Email Add-in Framework. This sample describes how to create and build a hosted email add-in provider, a log monitor, additions to the Dashboard UI, as well as a custom Windows Installer package. The sample also includes a simulated email service, which the provider links to in order to demonstrate simple email features.

Related topics
--------------

[Windows Dev Center](%20http://go.microsoft.com/fwlink/p/?linkid=302084)

[Windows Server Essentials](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513958)

[Working with a Hosted Email Service](http://msdn.microsoft.com/en-us/library/windows/desktop/jj991858)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Confirm that you have a Windows 2012 R2 Server with the Essentials Experience role enabled.
2.  Use Windows Explorer to navigate to the **%WinDir%\\Microsoft.NET\\assembly\\GAC\_MSIL** directory, and locate the following files:

    -   HomeAddinContract.dll
    -   Microsoft.windowsserversolutions.administration.objectmodel.dll
    -   Wssg.HostedEmailBase.dll
    -   Wssg.HostedEmailObjectModel.dll
    -   AdminCommon.dll
    -   ProviderFrameworkExtended.dll
    -   ProviderFramework.dll

3.  Copy these files into the **\\Library** directory under the sample directory.
4.  Confirm that you have the WIX toolset installed on your system. For more information, see [How to: Install the Windows Installer XML (WiX) Tools](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513936).
5.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
6.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
7.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

-   For information on how to set up and run the sample, see [Quickstart: Creating a Hosted Email Adapter](http://msdn.microsoft.com/en-us/library/windows/desktop/jj991886).

