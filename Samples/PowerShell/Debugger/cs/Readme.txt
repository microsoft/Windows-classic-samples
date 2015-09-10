PowreShell Debugging API Sample
===============================
     This sample shows how to use the PowerShell debugging API to handle debugger events and process debug
     commands.  It also demonstrates Workflow debugging.  Workflow debugging is new for PowerShell version 4 
     and is an opt-in feature.  In order to debug Workflow script functions the debugger DebugMode must include
     the DebugModes.LocalScript flag.  Note that Workflow is not supported for WOW (Windows on Windows) and so this 
     sample should be built using x64 CPU if building on an x64 platform. 
     The DebuggerSample class contains a single public Run method that creates a sample script file, sets
     a single breakpoint in the script Workflow function, and runs the script.  There are two event handlers
     that handle the debugger BreakpointUpdated and DebuggerStop events.  There is also a simple Read, 
     Evaluate, Print Loop (REPL) that handles user debugger commands.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a sample script file to debug that includes a Workflow function.
     2. How to handle a Debugger.BreakpointUpdated event.
     3. How to handle a Debugger.DebuggerStop event.
     4. How to process user debugger commands in a simple REPL.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. If you are building on an x64 platform make sure your are building to an x64 CPU.
     2. Open Windows Explorer and navigate to the Debugging\Debugger directory under the samples directory.
     3. Double-click the icon for the DebuggerSample.sln (solution) file to open the file in Visual Studio.
     4. Make sure the DebuggerSample project references the System.Management.Automation.dll assembly.
     5. In the Build menu, select Build Solution.
     6. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1.  Build the project as described above and run it.
     2.  A console will appear that runs the sample script file and stops at the breakpoint on line 10.
     3.  Type 'h' to get the debugger help list.
     4.  Type 'k' to see the call stack.
     5.  Type 'list' to see the script and the current line of execution.
     6.  Type 'v' to step through the Workflow script.
     7.  Type '$Title' to see the Title variable.
     8.  Type '$JobId' to see the Workflow job id.
     9.  Type 'Get-PSBreakpoint' to see the breakpoints.  You can also add a new breakpoint with Set-PSBreakpoint.
     10. Type 'o' to step out of the Workflow function.
     11. Type 'c' to finish running the script.
