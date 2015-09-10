Windows PowerShell Host06 Sample
================================

This sample application shows building a fairly complete interactive console-based host which allows users to run cmdlets and see the output.

This sample reads commands from the command line, executes them, and displays the results to the console. The sample user interface classes are built on top of System.Console.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating your own host interface, host user interface, and host raw user interface classes.
2.  Building a console application that uses these host classes to build an interactive PowerShell shell.
3.  Creating the \$profile variable and loading the four different profiles:
    1.  Current user, current host.
    2.  Current user, all hosts.
    3.  All users, current host.
    4.  All users, all hosts.

4.  Implementing the [**IHostUISupportsMultipleChoiceSelection**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144514) interface.
5.  Implementing the [**IHostSupportsInteractiveSession**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144513) interface to support interactive remoting via the Enter-PSSession and Exit-PSSession cmdlets.
6.  Using the [**PSParser.Tokenize**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd127763) method to colorize the command line as it is typed.

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

