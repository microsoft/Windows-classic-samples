Windows PowerShell Runspace 10 Sample
=====================================

This sample shows how to add a cmdlet to an InitialSessionState object and then use the modified InitialSessionState object when creating a Runspace object.

**Sample Objectives**

This sample describes the following:

1.  Creating an [**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569) object.
2.  Adding a cmdlet to the [**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569) object.
3.  Creating a runspace that uses the [**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569) object.
4.  Creating a [**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526) object that uses the Runspace object.
5.  Running the pipeline of the [**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526) object synchronously.
6.  Working with [**PSObject**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms572584) objects to extract properties from the objects returned by the pipeline.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569)

[**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526)

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

