Windows PowerShell Event Notification Sample
============================================

This sample shows how to receive notifications of Windows PowerShell events that are generated on remote computers.

**Sample Objectives**

This sample demonstrates the following:

-   How to use the [**PSEventReceived**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182209) event to receive notification of Windows PowerShell events that are generated on remote computers.

Related topics
--------------

**Conceptual**

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

**Reference**

[**PSEventReceived**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182209)

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

1.  Verify that Windows PowerShell remoting is enabled; you can run the the following command for additional information about how to enable this feature: `help about_remote`.
2.  Start the command prompt.
3.  Navigate to the folder containing the sample executable.
4.  Run the executable.
5.  See the output results and the corresponding code.

