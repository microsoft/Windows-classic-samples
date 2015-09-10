Windows PowerShell Host03 Sample
================================

This sample application shows building a fairly complete interactive console-based host which allows users to run cmdlets and see the output.

This sample reads commands from the command line, executes them, and displays the results to the console. The sample user interface classes are built on top of System.Console.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating your own host interface, host user interface, and host raw user interface classes.
2.  Building a console application that uses these host classes to build an interactive PowerShell shell.

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

