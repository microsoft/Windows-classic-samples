Windows PowerShell Activity Generator Sample
============================================

This sample shows how to use the Windows PowerShell Activity Generation API to generate the source code for an activity that wraps a Windows PowerShell command from C\# code. The sample then and then compiles the source code into an activity dll.

The sample uses the [**System.Management.Automation.PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms569889) API to get the definition of a command before calling the activity generation API. You can also accomplish this by calling the activity generator API from PowerShell directly, using the output of Get-Command as the command definition.

After you have generated the source code for an activity, compile it into a DLL and then reference that DLL from your Windows Workflow project.

The sample contains the `New-PSActivity.ps1` script that can be used to generate the source code for one or more activities and to compile the source into a single activity dll.

**Sample Objectives**

This sample demonstrates the following:

-   How to use the Windows PowerShell Activity Generation API to generate the source code for an activity that wraps a Windows PowerShell command from C\# code and then compile the source code into an activity dll.

Related topics
--------------

**Overview**

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

**Reference**

[**System.Management.Automation.PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms569889)

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
3.  Make sure the project references the System.Management.Automation.dll assembly.
4.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

**Running the sample**

1.  Start PowerShell and run the ActivityGenerator.exe program.
2.  If you wish to generate activities for a specific module, supply the module name as the first command-line parameter.

**Generating the Source Code and Activity DLL**

1.  Start PowerShell.
2.  Run the following command: .`\New-PSActivity.ps1 -Namespace ActivityGenerator.Test -ModulePath .\Math.cdxml -OutputPath .\ -AssemblyName ActivityGenerator.Test.Activities.dll -verbose`
3.  Reference `ActivityGenerator.Test.Activities.dll` DLL from your Windows Workflow project and create a workflow.

