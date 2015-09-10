Windows PowerShell StopProcessSample04 Sample
=============================================

This sample shows how to declare parameter sets, the input object, and how to specify the default parameter set to use. This cmdlet stops processes running on the local computer.

**Sample Objectives**

This sample demonstrates the following:

1.  Declaring a cmdlet class that derives from [**PSCmdlet**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551396)
2.  Declaring cmdlet parameters and parameter aliases.
3.  Specifying positions for parameters.
4.  Specifying that the parameters can accept an object from the pipeline or accept a value from a property of an object that has the same name as the parameter.
5.  Specifying parameter sets.
6.  Handling errors and exceptions.
7.  sing the [**ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256) and [**ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255) methods.
8.  Implementing the *Force* and *PassThru* parameters.
9.  Declaring aliases and wildcard support.
10. Implementing user notifications using [**WriteWarning**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568374), [**WriteVerbose**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568373), and [**WriteDebug**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568368).
11. Using InputObject.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**cmdlet**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms582518)

[**ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256)

[**ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255)

[**WriteWarning**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568374)

[**WriteVerbose**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568373)

[**WriteDebug**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568368)

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

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/StopProcessSample04**
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module StopProcessSample04` (This command loads the assembly into Windows PowerShell.)
4.  Type the following command to run the `cmdlet: Stop-Proc <process name>`

