Windows PowerShell GetProcessSample02 Sample
============================================

This sample shows how to create a cmdlet that has a parameter. The cmdlet takes one or more process names, and returns the matching processes.

**Sample Objectives**

This sample demonstrates the following:

1.  Declaring a cmdlet class.
2.  Declaring a parameter.
3.  Specifying the position of the parameter.
4.  Declaring a validation attribute for the parameter input.

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

1.  Store the assembly in the following module folder: `[user]/Documents/WindowsPowershell/Modules/GetProcessSample02`
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module GetProcessSample02` (This command loads the assembly into Windows PowerShell.)
4.  Now type the following command to run the cmdlet: `Get-Proc`

