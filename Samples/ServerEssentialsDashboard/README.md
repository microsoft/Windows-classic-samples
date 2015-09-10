Windows Server Essentials Dashboard add-in Samples
==================================================

This sample demonstrates how to develop various add-ins to the Windows Server Essentials Dashboard.

The Windows Server Essentials Dashboard is a UI designed to help simplify complex administrative tasks, and provide a consistent experience for an administrator. The Dashboard also exposes an API that allows 3rd party developers to add in their own functionality. This extended sample describes a number of ways a developer can add or modify Dashboard features, as described in the list below. Note that many of these tasks are also described in the Windows Server Essentials SDK documentation.

**Sample Objectives**

This sample demonstrates the following:

1.  Adding a Common Task
2.  Adding a Community Link
3.  Adding a Quick Status
4.  Adding a listview
5.  Adding a WinForm Control
6.  Adding a WPF Control
7.  Extended a Tab
8.  Adding a top-level Tab with Listview
9.  Adding a top-level tab with multiple subtabs
10. Adding a top-level tab with WinForm control
11. Adding a top-level tab with WPF control and extended user tabs

Related topics
--------------

[Windows Dev Center](%20http://go.microsoft.com/fwlink/p/?linkid=302084)

[Windows Server Essentials](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513958)

[Creating a Dashboard Add-In](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513895)

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

3.  Copy these two files into the **\\Library** directory under the sample directory.
4.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
5.  Go to the sample directory. Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
6.  Press F7 or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

The dashboard sample contain a number of dashboard add-ins that you can run on your Windows Server Essentials system. Generally, you can run each sample by taking the generated add-in (.addin) file and placing the file in the **%Program Files%\\Windows Server\\Bin\\Addins\\Users** directory. You may need to to re-start Server Manager to see the new additions. For more information, see the [Creating a Dashboard Add-In](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513895) topic and sub-topics. The sample also contains readme files that describe how to access certain add-in features.

