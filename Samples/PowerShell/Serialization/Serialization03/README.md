Windows PowerShell Serialization 03 Sample
==========================================

This sample looks at an existing .NET class and shows how to make sure that instances of this class and of derived classes are deserialized (rehydrated) into live .NET objects.

**Sample Objectives**

This sample demonstrates the following:

1.  Setting serialization depth for a given .NET class.
2.  Creating a type converter that can rehydrate a deserialized property bag into a live .NET object.
3.  Declaring that deserialized property bags of a given .NET class need to be rehydrated using the type converter.

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

1.  Start a Command Prompt.
2.  Navigate to the folder containing the sample binaries.
3.  Run Serialization02.exe.

