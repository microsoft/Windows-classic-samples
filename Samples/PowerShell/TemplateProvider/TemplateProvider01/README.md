Windows PowerShell Template Provider 01 Sample
==============================================

This sample creates a template for a provider that hooks into the Windows PowerShell namespaces. It contains all possible provider overrides and interfaces.

A provider developer can copy this file, change the name of the file, delete those interfaces and methods the provider doesn't need to implement or override, and use the remaining code as a template to create a fully functional provider.

**Sample Objectives**

This sample demonstrates the following:

-   How to create a TemplateProvider class that implements the following interfaces:
    1.  [**NavigationCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551375)
    2.  [**IPropertyCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551365)
    3.  [**IContentCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551352)
    4.  [**IDynamicPropertyCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551362)
    5.  [**ISecurityDescriptorCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551369)

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[**NavigationCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551375)

[**IPropertyCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551365)

[**IContentCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551352)

[**IDynamicPropertyCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551362)

[**ISecurityDescriptorCmdletProvider**](http://msdn.microsoft.com/en-us/library/windows/desktop/ms551369)

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

1.  Start a Command Prompt.
2.  Navigate to the folder containing the sample executable.
3.  Run the executable.
4.  See the output results and the corresponding code.

