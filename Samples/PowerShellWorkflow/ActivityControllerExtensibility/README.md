---
page_type: sample
languages:
- csharp
products:
- windows-api-win32
name: Activity Controller Extensibility sample
urlFragment: activity-controller-extensibility
description: Demonstrates how to extend the Windows PowerShell Workflow activity controller.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Activity controller extensibility sample

This sample demonstrates how to extend the Windows PowerShell Workflow activity controller.

The extended activity controller uses RunspacePool-based queuing on a local WinRM custom workflow endpoint. The sample includes the *CustomWorkflowEndpointSetup.ps1* script that can be used to set up a custom workflow endpoint.

## Sample Objectives

This sample demonstrates the following:

1.  How to extend the Windows PowerShell Workflow activity controller.
2.  How to create a custom workflow endpoint

## Related topics

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

## Operating system requirements

### Client

Windows 8.1

#### Server

Windows Server 2012 R2

## Build the sample

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (*.sln*) file.
3.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The *ActivityControllerExtensibilitySample.exe* file will be built in the *\\bin\\Debug* directory.

## Run the sample

1.  Start Windows PowerShell as Administrator.
2.  Navigate to the folder containing the *CustomWorkflowEndpointSetup.ps1* script.
3.  Run the *CustomWorkflowEndpointSetup.ps1* script to create a custom endpoint.
4.  Navigate to the folder containing the sample binaries.
5.  Run *ActivityControllerExtensibilitySample.exe*.

## Related topics

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)