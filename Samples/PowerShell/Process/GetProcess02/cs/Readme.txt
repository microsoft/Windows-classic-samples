Get-Process Sample 02
=====================
     This sample shows how to create a cmdlet that has a parameter. The cmdlet 
     takes one or more process names and returns the matching processes.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Declaring a cmdlet class.
     2. Declaring a parameter.
     3. Specifying the position of the parameter.
     4. Declaring a validation attribute for the parameter input.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to GetProcessSample02 under the 
        samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file 
        in Visual Studio.
     3. In the Build menu, select Build Solution.
        The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowershell/Modules/GetProcessSample02 
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module GetProcessSample02
        (This command loads the assembly into Windows PowerShell.)
     4. Now type the following command to run the cmdlet: Get-Proc
