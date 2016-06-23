Stop-Process Sample 02
======================
     This sample shows how to write a cmdlet that implements user notifications using
     WriteWarning, WriteVerbose, and WriteDebug. The cmdlet stops a specified process.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Declaring a cmdlet class.
     2. Declaring cmdlet parameters.
     3. Specifying positions for parameters.
     4. Specifying that the parameters can accept an object from the pipeline 
        or accept a value from a property of an object that has the same name 
        as the parameter.
     5. Handling errors and exceptions.
     6. Using the ShouldProcess and ShouldContinue methods.
     7. Implementing the Force and PassThru parameters.
     8. Implementing user notifications using WriteWarning, WriteVerbose and WriteDebug


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to StopProcessSample02 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/StopProcessSample02
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module StopProcessSample02
        (This command loads the assembly into Windows PowerShell.)
     4. Type the following command to run the cmdlet: Stop-Proc <process name>
