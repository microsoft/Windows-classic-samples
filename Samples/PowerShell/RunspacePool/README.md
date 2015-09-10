Windows PowerShell PowerShell02 Sample
======================================

This sample uses a [**RunspacePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182576) to run multiple commands concurrently. Although commands can be run synchronously using runspace pools, typically runspace pools are used to run commands concurrently.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a [**RunspacePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182576) with a minimum and maximum number of Runspaces.
2.  Creating many PowerShell commands with the same [**RunspacePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182576)
3.  Running the commands concurrently.
4.  Calling [**GetAvailableRunspaces**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd127653) to see how many Runspaces are free.
5.  Capturing the command output with [**EndInvoke.**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182447)

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**RunspacePool**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182576)

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

1.  Start command prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.
4.  See the output results and the corresponding code.

