AccessDB Provider Sample 01
===========================
     This sample shows how to implement a basic Windows PowerShell provider. Subsequent
     provider samples show how to implement additional provider functionality. Windows
     PowerShell providers enable users to access data in a consistent format that
     resembles a file system drive.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a basic Windows PowerShell provider.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to AccessDBProviderSample01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start Windows PowerShell.
     2. Navigate to the folder containing the sample DLL.
     3. Run Import-Module .\AccessDBProviderSample01.dll. This will add the AccessDB
        provider to the current environment.
     4. Run Get-PSProvider to see the new AccessDB provider.
