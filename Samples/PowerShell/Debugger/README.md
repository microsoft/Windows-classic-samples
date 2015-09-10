Windows PowerShell Debugger API Sample
======================================

This sample shows how to use the PowerShell Debugger API.

This sample shows how to use the PowerShell debugging API to handle debugger events and process debug commands. It also demonstrates Workflow debugging. Workflow debugging is new for PowerShell version 4 and is an opt-in feature. In order to debug Workflow script functions the debugger DebugMode must include the DebugModes.LocalScript flag. Note that Workflow is not supported for WOW (Windows on Windows) and so this sample should be built using x64 CPU if building on an x64 platform.

The DebuggerSample class contains a single public Run method that creates a sample script file, sets a single breakpoint in the script Workflow function, and runs the script. There are two event handlers that handle the debugger [**Debugger.BreakpointUpdated**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182204) and [**Debugger.DebuggerStop**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182205) events. There is also a simple Read, Evaluate, Print Loop (REPL) that handles user debugger commands.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a sample script file to debug that includes a Workflow function.
2.  How to handle a [**Debugger.BreakpointUpdated**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182204) event.
3.  How to handle a [**Debugger.DebuggerStop**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182205) event.
4.  How to process user debugger commands in a simple REPL.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

**Reference**

[**Debugger.BreakpointUpdated**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182204)

[**Debugger.DebuggerStop**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182205)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  If you are building on an x64 platform make sure your are building to an x64 CPU.
2.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
3.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
4.  Make sure the DebuggerSample project references the System.Management.Automation.dll assembly.
5.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Build the project as described above and run it.
2.  A console will appear that runs the sample script file and stops at the breakpoint on line 10.
3.  Type `'h'` to get the debugger help list.
4.  Type `'k`' to see the call stack.
5.  Type `'list'` to see the script and the current line of execution.
6.  Type `'v'` to step through the Workflow script.
7.  Type `'$Title'` to see the Title variable.
8.  Type `'$JobId'` to see the Workflow job id.
9.  Type `'Get-PSBreakpoint'` to see the breakpoints. You can also add a new breakpoint with Set-PSBreakpoint.
10. Type `'o'` to step out of the Workflow function.
11. Type `'c'` to finish running the script.

