Windows PowerShell Runspace 03 Sample
=====================================

This sample uses the PowerShell class to execute a script that retrieves process information for the list of process names passed into the script. It shows how to pass input objects to a script and how to retrieve error objects as well as the output objects.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating an instance of the PowerShell class.
2.  Using this instance to execute a string as a PowerShell script.
3.  Passing input objects to the script from the calling program.
4.  Using [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) to extract and display properties from the objects returned by this command.
5.  Retrieving and displaying error records generated during execution of the script.

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

