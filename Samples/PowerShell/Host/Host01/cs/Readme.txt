Host Sample 01
==========================
     This sample uses the RunspaceInvoke class to execute a script that 
     calls exit. The host application looks at this and prints out the 
     result.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating your own host interface class derived from PSHost.
        This example host implementation is mostly stubs but
        does illustrate mapping the exit status and exit code
        from the engine back to the calling program.
     2. Creating a runspace using that host implementation.
     3. Running a script then calling exit.
     4. Verifing that the script returned the correct value
        and the exit process operated correctly.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Host01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start command prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable
