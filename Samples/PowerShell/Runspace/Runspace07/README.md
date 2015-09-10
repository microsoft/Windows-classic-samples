Winodws PowerShell Runspace 07 Sample
=====================================

This sample shows how to create a runspace and how to run commands using a PowerShell object. It builds a pipeline that runs the `Get-Process` cmdlet, which is piped to the `Measure-Object` cmdlet to count the number of processes running on the system.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a runspace using the RunspaceFactory class.
2.  Creating a PowerShell object
3.  Adding individual cmdlets to the PowerShell object.
4.  Running the cmdlets synchronously.
5.  Working with [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract properties from the objects returned by the cmdlets.

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

