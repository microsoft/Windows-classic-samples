Host Sample 02
==========================
     A sample application that uses the PowerShell runtime along with a host
     implementation to call get-process and display the results as you
     would see them in pwrsh.exe. This sample adds the user interface classes
     to the basic host implementation classes from host sample 01.
     The sample user interface classes are built on top of System.Console.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to Host02 under the samples directory.
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

     1. Creating your own host interface, host user interface and
        host raw user interface classes.
     2. Creating a runspace using that host implementation.
     3. Setting the host curture.
     4. Running a script then calling exit.
     5. Verifing that the script returned the correct value
        and the exit process operated correctly.

    
