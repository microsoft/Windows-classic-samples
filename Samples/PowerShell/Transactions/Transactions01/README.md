Windows PowerShell Transacted Comment Sample
============================================

This sample shows a set of cmdlets that participate in Windows PowerShell transactions. These cmdlets provide a comment log that can be changed, completed, or rolled back along with the rest of the transaction. It also provides a simple transactional resource manager that stores a string.

**Sample Objectives**

This sample demonstrates the following:

1.  Usage of SupportsTransactionsAttribute.
2.  Usage of CurrentPSTransaction.
3.  Implementation of a .NET transacted resource manager.

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

1.  Store the assembly in the following module folder: **[user]/Documents/WindowsPowerShell/Modules/TransactedComment**
2.  Start Windows PowerShell.
3.  Run the following command: `Import-Module TransactedComment         `(This command loads the assembly into Windows PowerShell.)

