Windows PowerShell Runspace 01 Sample
=====================================

This sample uses the PowerShell class to run the `Get-Process` cmdlet synchronously. The **Name** and **HandleCount** are extracted from the objects returned by the cmdlet and displayed. Subsequent runspace samples show how to implement additional runspace functionality.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a PowerShell object to run a command.
2.  Adding a command to the pipeline of the PowerShell object.
3.  Running the command synchronously.
4.  Using [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract properties from the objects returned by the command.

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

