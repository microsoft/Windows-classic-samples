Windows PowerShell StopProcessSample02 Sample
=============================================

This sample shows how to write a cmdlet that implements user notifications using [**WriteWarning**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568374), [**WriteVerbose**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568373), and [**WriteDebug**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568368). The cmdlet stops a specified process.

**Sample Objectives**

This sample describes the following:

1.  Declaring a [**cmdlet**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms582518) class.
2.  Declaring cmdlet parameters.
3.  Specifying positions for parameters.
4.  Specifying that the parameters can accept an object from the pipeline or accept a value from a property of an object that has the same name as the parameter.
5.  Handling errors and exceptions.
6.  Using the [**ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256) and [**ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255) methods.
7.  Implementing the *Force* and *PassThru* parameters.
8.  Implementing user notifications using [**WriteWarning**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568374), [**WriteVerbose**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568373), and [**WriteDebug**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms568368).

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

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/StopProcessSample02**
2.  Start Windows PowerShell.
3.  Run the following command:` Import-Module StopProcessSample02` (This command loads the assembly into Windows PowerShell.)
4.  Type the following command to run the cmdlet: `Stop-Proc <process name>`

