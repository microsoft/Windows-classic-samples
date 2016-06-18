AccessDB Provider Sample 06
===========================
     This sample shows how to implement a basic Windows PowerShell provider that creates
     PowerShell drives to manipulate items, navigate containers, and access content. Windows PowerShell
     providers enable users to access data in a consistent format that resembles a file system drive.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a basic Windows PowerShell provider.
     2. Creating Windows PowerShell drives.
     3. Manipulating items through the provider.
     4. Manipulating containers of items.
     5. Providing support for levels of navigation.
     6. Accessing content.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to AccessDBProviderSample06 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start Windows PowerShell.
     2. Navigate to the folder containing the sample DLL.
     3. Run Import-Module .\AccessDBProviderSample06.dll. This will add the AccessDB
        provider to the current environment.
     4. Run Get-PSProvider to see the new AccessDB provider.
     5. Run Get-PSDrive to see the new PowerShell drive.
     6. Run Get-Item and Set-Item to manipulate items using the provider.
     7. Run Get-Location and Set-Location to navigate using the provider.
     8. Run Get-Content and Set-Content to access content using the provider.
