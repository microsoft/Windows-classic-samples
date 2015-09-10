Windows Server Essentials Launchpad add-in samples
==================================================

This sample shows how to add links to the Launchpad client agent for Windows Server Essentials.

The Launchpad is an application that displays a list of tasks that are organized in categories. You can extend the functionality of the Launchpad by adding your own tasks and categories, as well as links.

Related topics
--------------

[Windows Dev Center](%20http://go.microsoft.com/fwlink/p/?linkid=302084)

[Windows Server Essentials](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513958)

[Creating a Launchpad Add-in](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513952)

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

Run the sample
--------------

After you create the .launchpad file, you must place the file in the **%ProgramFiles%\\Windows Server\\Bin\\LaunchPad** directory of the target computer. You must then restart the Launchpad to completely install the add-in. For more information, see [Creating a Launchpad Add-in](http://msdn.microsoft.com/en-us/library/windows/desktop/gg513952).

