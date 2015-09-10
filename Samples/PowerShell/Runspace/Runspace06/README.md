Windows PowerShell Runspace 06 Sample
=====================================

This sample shows how to add a module to an initial session state, how to create a runspace that uses the initial session state, and how to run a cmdlet that is provided by the module. This sample assumes that the user has the `GetProcessSample02.dll` that is produced by the GetProcessSample02 sample copied to the current directory.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a default initial session state.
2.  Creating a runspace that uses the initial session state.
3.  Creating a PowerShell object that uses the runspace.
4.  Adding a cmdlet of a module to the PowerShell object.
5.  Using [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract and display properties from the objects returned by the cmdlet.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584)

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

1.  Start a Command Prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.

