Windows PowerShell Serialization 02 Sample
==========================================

By default, serialization preserves all public properties of an object. This sample looks at an existing .NET class and shows how to make sure that information from an instance of this class is preserved across serialization and deserialization when the information is not available in public properties of the class.

**Sample Objectives**

The sample demonstrates the following:

1.  Setting serialization depth for a given .NET class.
2.  Adding an extended property that carries information not available in public properties of the .NET class.
3.  Hiding the extended property (an implementation detail needed for serialization) from users.

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

