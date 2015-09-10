Windows PowerShell Select-Object 01 Sample
==========================================

This sample creates a cmdlet called `Select-Obj` which acts as a filter to select only certain objects to process or pass down the pipeline. It is most effectively used as a pipeline receiver from other cmdlets such as `Get-Service` or `Get-Process`. The *First*, *Last*, and *Unique* parameters select which objects to process. The cmdlet works with files, modules, registry keys, and other objects.

**Sample Objectives**

This sample demonstrates the following:

-   Creating an advanced cmdlet.

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

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/SelectObjSample01**
2.  Start Windows PowerShell.
3.  Run the following command: Import-Module SelectObjSample01 (This command loads the assembly into Windows PowerShell.)
4.  Type the following command to run the cmdlet: Select-Obj

