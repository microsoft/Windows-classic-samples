Windows PowerShell Remote Runspace 01 Sample
============================================

This sample shows how to construct a remote runspace, set connection and operation timeouts, establish a remote connection, and close the connection.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating an instance of the [**WSManConnectionInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182592) class.
2.  Setting [**OperationTimeout**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd536113) and [**OpenTimeOut**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd536112) using this [**WSManConnectionInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182592) instance.
3.  Using this [**WSManConnectionInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182592) instance to establish a remote connection.
4.  Closing the remote connection.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**WSManConnectionInfo**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182592)

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

    The executable will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Verify that Windows PowerShell remoting is enabled. You can run the following command for additional information about how to enable this feature: `help about_remote`.
2.  Start the Command Prompt as Administrator.
3.  Navigate to the folder containing the sample executable.
4.  Run the executable.
5.  See the output results and the corresponding code.

