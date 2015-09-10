Windows PowerShell Runspace 8 Sample
====================================

This sample shows how to use a PowerShell object to run commands. The PowerShell object builds a pipeline that include the `Get-Process` cmdlet, which is then piped to the `Sort-Object` cmdlet. Parameters are added to the `Sort-Object` cmdlet to sort the **HandleCount** property in descending order.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a PowerShell object.
2.  Adding individual commands to the PowerShell object.
3.  Adding parameters to the commands.
4.  Running the commands synchronously.
5.  Working with [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract properties from the objects returned by the commands.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584)

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

1.  Start a Command Prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.

