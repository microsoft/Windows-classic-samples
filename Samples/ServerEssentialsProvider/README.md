Windows Server Essentials Provider Samples
==========================================

The Provider Framework is a communication framework in Windows Server Essentials that allows a developer to develop server management components. This sample shows how to create a provider to create a chat room and how to interact between the UI and the provider.

For more information about running the ChatProvider sample, see [Creating a Provider](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513899). This topic is a mult-step walkthrough, based on this sample, for building a Windows Server Essentials provider.

Related topics
--------------

[Windows Dev Center](%20http://go.microsoft.com/fwlink/p/?linkid=302084)

[Windows Server Essentials](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513958)

[Creating a Provider](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513899)

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

    -   ProviderFramework.dll

3.  Copy these files into the **\\Library** directory under the sample directory.
4.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
5.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
6.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

After you have created the provider files, you must install the provider files by copying the assemblies to folders on the target server.

1.  Copy the .exe file for the ChatWindow project from the **bin\\Debug** folder to **%Program Files%\\Windows Server\\Bin**.
2.  Copy the .dll files for the ChatSample.ObjectModel project from the **bin\\Debug** folder to **%Program Files%\\Windows Server\\Bin**.
3.  Copy the .exe for the ChatSample project from the **bin\\Debug** folder to **%Program Files%\\Windows Server\\Bin**.
4.  Run the ChatSample.exe file and then run one or more copies of the ChatWindow.exe file.

