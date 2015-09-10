Windows PowerShell Runspace 09 Sample
=====================================

This sample shows how to use a PowerShell object to run a script that generates the numbers from 1 to 10 with delays between each number. The pipeline of the PowerShell object is run asynchronously and events are used to handle the output.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a [**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526) object.
2.  Adding a script to the pipeline of the [**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526) object.
3.  Using the [**BeginInvoke**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd127743) method to run the pipeline asynchronosly.
4.  Using the events of the [**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526) object to process the output of the script.
5.  Using the [**PowerShell.Stop**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182457) method to interrupt an executing pipeline.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**PowerShell**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144526)

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

