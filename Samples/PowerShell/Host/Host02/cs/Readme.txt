Host Sample 02
==========================
     This sample application uses the PowerShell runtime along with a host
     implementation to call Get-Process and display the results as you
     would see them in powershell.exe. This sample adds the user interface 
     classes to the basic host implementation classes from Host Sample 01.
     The sample user interface classes are built on top of System.Console.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating your own host interface, host user interface, and
        host raw user interface classes.
     2. Creating a runspace using that host implementation.
     3. Setting the host culture.
     4. Running a script and then calling exit.
     5. Verifing that the script returned the correct value
        and exiting the process operated correctly.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Host02 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start command prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable
