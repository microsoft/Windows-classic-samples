Runspace Sample 01
==========================
     This sample uses the RunspaceInvoke class to execute
     the get-process cmdlet synchronously. The name and
     handlecount are then extracted from  the PSObjects
     returned and displayed.

Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - Visual Basic

To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to Runspace01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The library will be built in the default \bin or \bin\Debug directory.

To run the sample:
=================
     1. Start command prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable

Demonstrates
============
     This sample demonstrates the following:

     1. Creating an instance of the RunspaceInvoke class.
     2. Using this instance to invoke a PowerShell command.
     3. Using PSObject to extract properties from the objects
        returned by this command.

