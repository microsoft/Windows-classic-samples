Supports Paging 01 Sample
=========================
     This sample shows how to implement a cmdlet called Get-Numbers that supports paging
     operations. The Get-Numbers cmdlet generates up to 100 consecutive numbers starting from 0.
     The IncludeTotalCount, Skip, and First parameters enable the user to perform paging operations
     on the set of numbers returned by the cmdlet.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Usage of the SupportsPaging attribute to implement paging functionality


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#
     - Windows PowerShell Script


Building the Sample Using Visual Studio
=======================================
     1. Open Windows Explorer and navigate to SupportsPaging01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the C# Sample
=====================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/SupportsPaging01
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module SupportsPaging01
        (This command loads the assembly into Windows PowerShell.)
     4. Run the Get-Numbers cmdlet.


Running the Windows PowerShell Script Sample
============================================
     1. Open Windows PowerShell
     2. Navigate to the directory where the SupportsPaging01.ps1 script is stored
     2. Dot source the script by running the following command:
        . .\SupportsPaging01.ps1


Using the Sample
================
     1. Get-Numbers -IncludeTotalCount
     2. Get-Numbers -NumbersToGenerate 0 -IncludeTotalCount
     2. Get-Numbers -NumbersToGenerate 90 -Skip 20 -IncludeTotalCount
     3. Get-Numbers -NumbersToGenerate 90 -Skip 20 -First 12 -IncludeTotalCount
