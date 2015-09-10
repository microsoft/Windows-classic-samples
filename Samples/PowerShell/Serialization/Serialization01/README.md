Windows PowerShell Serialization 01 Sample
==========================================

This sample looks at an existing .NET class and shows how to make sure that information from selected public properties of this class is preserved across serialization and deserialization. The sample uses a types.ps1xml file to declare which properties of the class should get serialized.

**Sample Objectives**

This sample demonstrates the following:

1.  Setting serialization depth for a given .NET class.
2.  Restricting which set of properties from a class are serialized.
3.  Demonstrating the effects of using a types.ps1xml file and [**DeserializingTypeConverter**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144399).

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**DeserializingTypeConverter**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144399)

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
3.  Run Serialization01.exe.

