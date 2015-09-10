Windows PowerShell Select-String Sample
=======================================

This sample creates a cmdlet called `Select-Str` that searches files for specified patterns. The patterns can be case-sensitive or case-insensitive. The user can also specify a script block to use for performing the matching operation instead of relying on the cmdlet's own logic which uses regular expressions for matching.

**Sample Objectives**

This sample demonstrates the following:

1.  Using PSPaths with cmdlet parameters.
2.  Using script blocks with cmdlet parameters.
3.  Using session state in a cmdlet implementation.

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

1.  Store the assembly in the following module folder: [user]/Documents/WindowsPowerShell/Modules/SelectStrCommandSample
2.  Start Windows PowerShell.
3.  Run the following command: Import-Module SelectStrCommandSample (This command loads the assembly into Windows PowerShell.)
4.  Type Get-Command Select-Str -Syntax to see the syntax for the Select-Str cmdlet.
5.  Type Select-Str to run the cmdlet.

