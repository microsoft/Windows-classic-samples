Windows PowerShell Supports Paging 01 Sample
============================================

This sample shows how to implement a cmdlet called `Get-Numbers` that supports paging operations. The `Get-Numbers` cmdlet generates up to 100 consecutive numbers starting from 0. The *IncludeTotalCount*, *Skip*, and *First* parameters enable the user to perform paging operations on the set of numbers returned by the cmdlet.

**Sample Objectives**

This sample demonstrates the following:

-   Usage of the SupportsPaging attribute to implement paging functionality.

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

    The library will be built in the default \\bin or \\bin\\Debug directory.

Run the sample
--------------

**Running the C\# Sample**

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/SupportsPaging01**
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module SupportsPaging01` (This command loads the assembly into Windows PowerShell.)
4.  Run the `Get-Numbers` cmdlet.

**Running the Windows PowerShell Script Sample**

1.  Open Windows PowerShell.
2.  Navigate to the directory where the SupportsPaging01.ps1 script is stored.

    By default, the script is located in the **.../PowerShell/SupportsPaging/SupportsPaging\_Script/CS** directory.

3.  Dot source the script by running the following command: `. .\SupportsPaging01.ps1`

