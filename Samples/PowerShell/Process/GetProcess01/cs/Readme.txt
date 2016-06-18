Get-Process Sample 01
==========================
     This sample shows how to create a simple cmdlet.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a basic sample cmdlet.
     2. Creating a snap-in that works with Windows PowerShell version 1.0 or greater.
        Note that subsequent samples will use modules (a Windows PowerShell version 2.0 feature)
        instead of snap-ins.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to GetProcessSample01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt window.
     2. Navigate to the folder containing the sample DLL.
     3. Run installutil "GetProcessSample01.dll".
     4. Start Windows PowerShell.
     5. Run the following command: Add-PSSnapin GetProcPSSnapIn01
        (This command adds the PowerShell snap-in to the shell.)
     6. Type the following command to run the cmdlet: Get-Proc
