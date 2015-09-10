Windows PowerShell Host02 Sample
================================

This sample application uses the PowerShell runtime along with a host implementation to call Get-Process and display the results as you would see them in powershell.exe.

This sample adds the user interface classes to the basic host implementation classes from Host Sample 01. The sample user interface classes are built on top of System.Console.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating your own host interface, host user interface, and host raw user interface classes.
2.  Creating a runspace using that host implementation.
3.  Setting the host culture.
4.  Running a script and then calling exit.
5.  Verifying that the script returned the correct value and exiting the process operated correctly.

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

1.  Start command prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.

