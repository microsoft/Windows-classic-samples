Stop-Process Sample 04
======================
     This sample shows how to declare parameter sets, the input object, and
     how to specify the default parameter set to use. This cmdlet stops 
     processes running on the local computer.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Declaring a cmdlet class that derives from PSCmdlet.
     2. Declaring cmdlet parameters and parameter aliases.
     3. Specifying positions for parameters.
     4. Specifying that the parameters can accept an object from the pipeline 
        or accept a value from a property of an object that has the same name 
        as the parameter.
     5. Specifying parameter sets.
     6. Handling errors and exceptions.
     7. Using the ShouldProcess and ShouldContinue methods.
     8. Implementing the Force and PassThru parameters.
     9. Declaring aliases and wildcard support.
     10. Implementing user notifications using WriteWarning, WriteVerbose and WriteDebug.
     11. Using InputObject.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to StopProcessSample04 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/StopProcessSample04
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module StopProcessSample04
        (This command loads the assembly into Windows PowerShell.)
     4. Type the following command to run the cmdlet: Stop-Proc <process name>
