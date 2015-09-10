Windows PowerShell PowerShell01 Sample
======================================

This sample uses [**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569) to constrain a runspace and add commands and providers.

This sample will concentrate on the SDK mechanisms to restrict the runspace. Script alternatives to the SDK include \$ExecutionContext.SessionState.LanguageMode and the PSSessionConfiguration cmdlets.

**Sample Objectives**

This sample demonstrates the following:

1.  Restricting the language by setting the [**InitialSessionState.LanguageMode**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144218) property.
2.  Adding aliases to the environment using [**InitialSessionState.Commands**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144216) property.
3.  Marking commands as private.
4.  Removing providers using [**InitialSessionState.Providers**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144219) property.
5.  Removing commands using [**InitialSessionState.Commands**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd144216) property.
6.  Adding commands and providers.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**InitialSessionState**](http://msdn.microsoft.com/en-us/library/windows/desktop/dd182569)

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

1.  Start command prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.
4.  See the output results and the corresponding code.

