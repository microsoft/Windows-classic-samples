Runspace Sample 01
==========================
     This sample uses the PowerShell class to run the Get-Process cmdlet synchronously. The Name
     and HandleCount are extracted from the objects returned by the cmdlet and displayed. Subsequent
     runspace samples show how to implement additional runspace functionality.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a PowerShell object to run a command.
     2. Adding a command to the pipeline of the PowerShell object.
     3. Running the command synchronously.
     4. Using PSObject objects to extract properties from the objects returned by the command.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#

Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
