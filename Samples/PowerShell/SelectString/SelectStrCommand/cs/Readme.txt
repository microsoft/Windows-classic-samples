Select-String Sample
====================
     This sample creates a cmdlet called Select-Str that searches files for specified patterns.
     The patterns can be case-sensitive or case-insensitive. The user can also specify a script
     block to use for performing the matching operation instead of relying on the cmdlet's own
     logic which uses regular expressions for matching.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Using PSPaths with cmdlet parameters
     2. Using script blocks with cmdlet parameters
     3. Using session state in a cmdlet implementation


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to SelectStringCommandSample under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/SelectStrCommandSample
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module SelectStrCommandSample
        (This command loads the assembly into Windows PowerShell.)
     4. Type Get-Command Select-Str -Syntax to see the syntax for the Select-Str cmdlet.
     5. Type Select-Str to run the cmdlet.
