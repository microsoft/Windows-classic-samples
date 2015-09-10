Windows Powershell role-based OData Web Service sample
======================================================

Using the Management OData Web Service requires a third party to implement the Microsoft.Management.OData.CustomAuthorization and System.Management.Automation.Remoting.PSSessionConfiguration interfaces to expose Windows PowerShell cmdlets. These interfaces perform user authorization and provide PowerShell session configuration. This sample shows an implementation of the two interfaces using a role-based authorization model. This model can define multiple roles. Each role is associated with a group of users. Each role is also associated with a set of cmdlets, scripts, and modules. A user can be assigned to only one of these roles, and that user can run only the set of cmdlets, scripts, and modules associated with that role.

**Important**  This sample requires Microsoft Visual Studio 2010, 2013, or later versions (any SKU). It doesn't compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Operating system requirements
-----------------------------

Client

None supported

Server

Windows Server 2012 R2

Build the sample
----------------

This sample must be built on a computer that is running Windows Server 2012 R2 with the **Management OData IIS Extension** feature installed. To install this feature:

1.  Open an elevated PowerShell window.
2.  Enter **Set-ExecutionPolicy -ExecutionPolicy Unrestricted**.
3.  Enter *TargetDirectory* **PswsRoleBasedPlugins\\C\#\\setup\\InstallModata.ps1**.

To build the sample using Visual Studio:

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled RoleBasedPlugins.sln.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

To build the sample from a Command Prompt window:

1.  Open a Command Prompt window and navigate to the sample directory.
2.  Enter **msbuild RoleBasedPlugins.sln**.

Run the sample
--------------

1.  Open an elevated PowerShell window.
2.  Navigate to the solution \\Debug or \\Release directory.
3.  Enter **.\\setupEndPoint.ps1**.

