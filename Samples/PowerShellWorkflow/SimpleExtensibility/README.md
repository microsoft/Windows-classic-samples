Simple Workflow Extensibility Sample
====================================

This sample demonstrates how to host the Windows PowerShell Worflow runtime in an application.

In the sample, the **OutOfProcessActivity**, **AllowedActivity**, and **LanguageMode** members of the **PSWorkflowConfigurationProvider** class are overridden with custom implementations. The custom configuration is passed as the configuration provider when the PSWorkflowRuntime is created. Finally, the runtime is used to invoke a sample workflow.

**Sample Objectives**

This sample demonstrates the following:

-   How to host the Windows PowerShell Workflow runtime in an application.

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

    The SimpleExtensibilitySample.exe file will be built in the **\\bin\\Debug** directory.

Run the sample
--------------

1.  Start a Command Prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.

