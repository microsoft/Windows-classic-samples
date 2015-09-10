Windows PowerShell Host01 Sample
================================

This sample uses the RunspaceInvoke class to execute a script that calls exit. The host application looks at this and prints out the result.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating your own host interface class derived from [**PSHost**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572439). This example host implementation is mostly stubs but does illustrate mapping the exit status and exit code from the engine back to the calling program.
2.  Creating a runspace using that host implementation.
3.  Running a script then calling exit.
4.  Verifying that the script returned the correct value and the exit process operated correctly.

Related topics
--------------

**Conceptual**

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

**Reference**

[**PSHost**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572439)

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

