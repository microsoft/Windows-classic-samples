get-process Sample 01
==========================
     This sample shows how to create a simple cmdlet. 


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to GetProcessSample01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The library will be built in the default \bin or \bin\Debug directory.


To run the sample:
=================
     1. Start a Command Prompt window.
     2. Navigate to the folder containing the sample dll.
     3. Run installutil "GetProcessSample01.dll"
     4. Start Windows PowerShell
     5. Run the following command: Add-PSSnapin GetProcPSSnapIn01
        (This command adds the PowerShell snap-in to the shell.)
     6. Now type the following command to run the cmdlet: get-proc
  

Demonstrates
============
     This sample demonstrates the following:
     1. Creating a basic sample cmdlet.
     2. Creating a snapin that works with both version 1 and version 2 of Windows PowerShell.
        Note that subsequent samples will use modules (a version 2 feature) instead of snapins.