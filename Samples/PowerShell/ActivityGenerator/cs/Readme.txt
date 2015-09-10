Activity Generator Sample
==========================
    This sample shows how to generate source code for Windows Workflow Activities that wrap 
    Windows PowerShell commands.
    
    The sample uses the System.Management.Automation.PowerShell API to get the definition of a
    command before calling the activity generation API. You can also accomplish this by calling
    the activity generator API from PowerShell directly, using the output of Get-Command as the
    command definition.

    After you have generated the source code for an activity, compile it into a DLL and then
    reference that DLL from your Windows Workflow project.

    The sample contains the New-PSActivity.ps1 script that can be used to generate the source
    code for one or more activities and to compile the source into a single activity dll.

    For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145 


Sample Objectives
=================
     This sample demonstrates the following:

     - How to use the Windows PowerShell Activity Generation API to generate the source code for an activity that
        wraps a Windows PowerShell command from C# code and then compile the source code into an activity dll.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open Windows Explorer and navigate to the ActivityGenerator directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. Make sure the project references the System.Management.Automation.dll assembly.
     4. In the Build menu, select Build Solution.
     5. The library file will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start PowerShell and run the ActivityGenerator.exe program.
     2. If you wish to generate activities for a specific module, supply the module name as the first command-line parameter.


Generating the Source Code and Activity DLL
===========================================
     1. Start PowerShell.
     2. Run the following command:
        .\New-PSActivity.ps1 -Namespace ActivityGenerator.Test -ModulePath .\Math.cdxml `
        -OutputPath .\ -AssemblyName ActivityGenerator.Test.Activities.dll -verbose
     3. Reference ActivityGenerator.Test.Activities.dll DLL from your Windows Workflow project and create a workflow.
