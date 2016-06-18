Host Sample 01
==========================
     This sample uses the RunspaceInvoke class to execute
     a script that calls exit. The host application looks at
     this and prints out the result.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to Host01 under the samples directory.
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

     1. Creating your own host interface class derived from PSHost.
        This example host implementation is mostly stubs but
        does illustrate mapping the exit status and exit code
        from the engine back to the calling program.
     2. Creating a runspace using that host implementation.
     3. Running a script then calling exit.
     4. Verifing that the script returned the correct value
        and the exit process operated correctly.

