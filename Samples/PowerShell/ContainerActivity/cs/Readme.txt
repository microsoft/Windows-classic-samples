Container Generator Sample
==========================
    This sample shows how to write a Windows Workflow Activity that can accept
	a PowerShell script block as an argument.

	The context of this sample is an activity that lets you invoke actions very cautiously -
	after invoking the action, the workflow suspends and asks for manual verification. If
	you are satisfied that the action was accomplished correctly, then you can delete the
	log file and resume the workflow. If you want the action to be attempted again, just
	resume the workflow without deleting the log file.
    
    The activity implements support for script block arguments by defining a property of type 'Activity'.
	When the user supplies a script block to this parameter, PowerShell automatically converts
	the script block into an activity graph - using the same mechanism that PowerShell already
	relies on to convert your workflow scripts into workflows that Windows Workflow Foundation
	can understand.

    Once you have compiled the ContainerActivity project, you can use the #requires statement to
	reference that DLL from a regular PowerShell Workflow.

    For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145 


Sample Objectives
=================
     This sample demonstrates the following:

     - How to write a Workflow activity that can accept a PowerShell script block as an argument,
	    when called from a PowerShell Workflow.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open Windows Explorer and navigate to the ContainerActivity directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. Make sure the project references the Microsoft.PowerShell.Activities.dll assembly.
     4. In the Build menu, select Build Solution.
     5. The library file will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start PowerShell
	 2. Navigate to the solution directory
	 3. Follow the steps demonstrated in the Invoke-ContainerActivity.ps1 sample script