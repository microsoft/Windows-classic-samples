Windows PowerShell GetProcessSample01 Sample
============================================

This sample shows how to create a simple cmdlet.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a basic sample cmdlet.
2.  Creating a snap-in that works with Windows PowerShell version 1.0 or greater.

    Note that subsequent samples will use modules (a Windows PowerShell version 2.0 feature) instead of snap-ins.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Start an elevated Command Prompt window.
2.  Navigate to the folder containing the sample DLL.
3.  Run `installutil "GetProcessSample01.dll"`

    > **Note**  On a 64-bit operating system, you must run the 64-bit version of installutil.

    .

4.  Start Windows PowerShell.
5.  Run the following command: `Add-PSSnapin GetProcPSSnapIn01` (This command adds the PowerShell snap-in to the shell.)
6.  Type the following command to run the cmdlet: `Get-Proc`

