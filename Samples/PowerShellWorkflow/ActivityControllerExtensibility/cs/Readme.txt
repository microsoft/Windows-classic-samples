Activity Controller Extensibility Sample
========================================
    This sample demonstrates how to extend the Windows PowerShell Workflow activity controller.
    The extended activity controller uses RunspacePool-based queuing on a local WinRM custom
    workflow endpoint. The sample includes the CustomWorkflowEndpointSetup.ps1 script that can be
    used to set up a custom workflow endpoint.

    For Windows PowerShell Workflow information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. How to extend the Windows PowerShell Workflow activity controller
     2. How to create a custom workflow endpoint


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to ActivityControllerExtensibilitySample under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The ActivityControllerExtensibilitySample.exe file will be built in the \bin\Debug directory.


Running the Sample
==================
     1. Start Windows PowerShell as Administrator.
     2. Navigate to the folder containing the CustomWorkflowEndpointSetup.ps1 script.
     3. Run the CustomWorkflowEndpointSetup.ps1 script to create a custom endpoint.
     4. Navigate to the folder containing the sample binaries.
     5. Run ActivityControllerExtensibilitySample.exe
