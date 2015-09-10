Windows PowerShell StopProcessSample01 Sample
=============================================

This sample shows how to write a cmdlet that implements a *PassThru* parameter that indicates that the user wants the cmdlet to return an object, and how to request user feedback by calls to the [**ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256) and [**ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255) methods. This cmdlet stops a specified process.

**Sample Objectives**

This sample describes the following:

1.  Declaring a cmdlet class.
2.  Declaring cmdlet parameters.
3.  Specifying positions for parameters.
4.  Specifying that the parameters can accept an object from the pipeline or accept a value from a property of an object that has the same name as the parameter.
5.  Handling errors and exceptions.
6.  Using the [**ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256) and [**ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255) methods.
7.  Implementing the *Force* and *PassThru* parameters.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**Cmdlet.ShouldProcess**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570256)

[**Cmdlet.ShouldContinue**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms570255)

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

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/StopProcessSample01**
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module StopProcessSample01` (This command loads the assembly into Windows PowerShell.)
4.  Type the following command to run the cmdlet:` Stop-Proc`

