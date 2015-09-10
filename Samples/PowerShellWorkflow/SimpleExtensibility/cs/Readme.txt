Simple Workflow Extensibility Sample
====================================
    This sample demonstrates how to host the Windows PowerShell Worflow runtime in an application.
    In the sample, the OutOfProcessActivity, AllowedActivity, and LanguageMode members of the
    PSWorkflowConfigurationProvider class are overridden with custom implementations. The custom
    configuration is passed as the configuration provider when the PSWorkflowRuntime is created.
    Finally, the runtime is used to invoke a sample workflow.

    For Windows PowerShell Workflow information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. How to host the Windows PowerShell Workflow runtime in an application.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to the SimpleExtensibilitySample directory under the
        samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The SimpleExtensibilitySample.exe file will be built in the \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
