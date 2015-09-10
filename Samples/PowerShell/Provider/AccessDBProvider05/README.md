Windows PowerShell AccessDB Provider Sample 05
==============================================

This sample shows how to implement a basic Windows PowerShell provider that creates PowerShell drives to manipulate items with containers and navigation support.

Subsequent provider samples show how to implement additional provider functionality. Windows PowerShell providers enable users to access data in a consistent format that resembles a file system drive.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a basic Windows PowerShell provider.
2.  Creating Windows PowerShell drives.
3.  Manipulating items through the provider.
4.  Manipulating containers of items.
5.  Providing support for levels of navigation.

Related topics
--------------

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

[Writing a Windows PowerShell Provider](http://msdn.microsoft.com/en-us/library/windows/desktop/ee126192(v=vs.85).aspx)

[Managing Windows PowerShell Drives](http://technet.microsoft.com/en-us/library/dd315335.aspx)

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

1.  Start Windows PowerShell.
2.  Navigate to the folder containing the sample DLL.
3.  Run Import-Module .\\AccessDBProviderSample05.dll. This will add the AccessDB provider to the current environment.
4.  Run Get-PSProvider to see the new AccessDB provider.
5.  Run Get-PSDrive to see the new PowerShell drive.
6.  Run Get-Item and Set-Item to manipulate items using the provider.
7.  Run Get-Location and Set-Location to navigate using the provider.

