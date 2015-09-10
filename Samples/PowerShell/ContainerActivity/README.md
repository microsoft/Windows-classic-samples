Windows PowerShell Container Activity Sample
============================================

This sample shows how to write a Windows Workflow Activity that can accept a PowerShell script block as an argument.

The context of this sample is an activity that lets you invoke actions very cautiously: after invoking the action, the workflow suspends and asks for manual verification. If you are satisfied that the action was accomplished correctly, then you can delete the log file and resume the workflow.

If you want the action to be attempted again, you can resume the workflow without deleting the log file. The activity implements support for script block arguments by defining a property of type 'Activity'. When the user supplies a script block to this parameter, PowerShell automatically converts the script block into an activity graph. The script uses the same mechanism that PowerShell already relies on to convert your workflow scripts into workflows that Windows Workflow Foundation can understand.

Once you have compiled the ContainerActivity project, you can use the \#requires statement to reference that DLL from a regular PowerShell Workflow.

**Sample Objectives**

This sample demonstrates the following:

-   How to write a Workflow activity that can accept a PowerShell script block as an argument, when called from a PowerShell Workflow.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[Getting Started with Windows PowerShell Workflow](http://technet.microsoft.com/en-us/library/jj134242.aspx)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Make sure the project references the `Microsoft.Powershell.Activities.dll` assembly.
4.  Press F7 or use **Build** \> **Build Solution** to build the sample.
5.  The library file will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Start **PowerShell**.
2.  Navigate to the solution directory.
3.  Follow the steps demonstrated in the `Invoke-ContainerActivity.ps1` sample script.

