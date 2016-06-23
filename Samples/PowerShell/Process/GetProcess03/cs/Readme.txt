Get-Process Sample 03
=====================
     This sample shows how to create a cmdlet that can take input from the pipeline.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Declaring a cmdlet class.
     2. Declaring a parameter.
     3. Specifying the position of the parameter.
     4. Specifying that the parameter takes input from the pipeline. The input can be taken 
        from an obect or taken from a property of an object that has the same name as 
        the parameter.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to GetProcessSample03 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/GetProcessSample03
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module GetProcessSample03
        (This command loads the assembly into Windows PowerShell.)
     4. Type the following command to run the cmdlet: Get-Proc
