Windows PowerShell Runspace 04 Sample
=====================================

This sample shows how to use the PowerShell class to run commands. The commands generate a terminating exception that the caller should catch and process.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a PowerShell object.
2.  Using the PowerShell object to run the commands.
3.  Passing input objects to the commands from the calling program.
4.  Using [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract and display properties from the objects returned by the commands.
5.  Retrieving and displaying error records that were generated while running the commands.
6.  Catching and displaying terminating exceptions generated while running the commands.

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

