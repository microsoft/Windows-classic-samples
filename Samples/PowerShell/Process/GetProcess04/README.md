Windows PowerShell GetProcessSample04 Sample
============================================

This sample shows how to create a cmdlet that handles non-terminating errors.

**Sample Objectives**

This sample demonstrates the following:

1.  Declaring a cmdlet class.
2.  Declaring a parameter.
3.  Specifying the position of a parameter.
4.  Specifying that the parameter can accept an object from the pipeline or accept a value from a property of an object that has the same name as the parameter.
5.  Trapping a non-terminating error and writing an error message to the error stream.

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

    The library will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Store the assembly in the following module folder: [`user]/Documents/WindowsPowerShell/Modules/GetProcessSample04`
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module GetProcessSample04` (This command loads the assembly into Windows PowerShell.)
4.  Type the following command to run the cmdlet: `Get-Proc`

