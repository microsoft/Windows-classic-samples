Runspace Sample 08
==================
     This sample shows how to use a PowerShell object to run commands. The PowerShell object builds
     a pipeline that include the Get-Process cmdlet, which is then piped to the Sort-Object cmdlet.
     Parameters are added to the Sort-Object cmdlet to sort the HandleCount property in descending
     order.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a PowerShell object.
     2. Adding individual commands to the PowerShell object.
     3. Adding parameters to the commands.
     4. Running the commands synchronously.
     5. Working with PSObject objects to extract properties from the objects returned by the commands.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace08 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
