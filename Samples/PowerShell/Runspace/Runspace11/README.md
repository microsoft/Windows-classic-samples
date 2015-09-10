Windows PowerShell Runspace 11 Sample
=====================================

This sample shows how to use the [**CommandMetadata**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144506) class to create a proxy command that calls an existing cmdlet, but restricts the set of available parameters. The proxy command (a function) is then added to an initial session state that is used to create a constrained runspace. The user can call the function, but cannot call not the initial cmdlet.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a [**CommandMetadata**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144506) object that describes the metadata of an existing cmdlet.
2.  Modifying the cmdlet metadata to remove a parameter from the cmdlet.
3.  Adding the cmdlet to an initial session state and making it private.
4.  Creating a proxy function that calls the existing cmdlet, but exposes only a restricted set of parameters.
5.  Adding the proxy function to the initial session state.
6.  Calling the private cmdlet and the proxy function to demonstrate the constrained runspace.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**CommandMetadata**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144506)

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

