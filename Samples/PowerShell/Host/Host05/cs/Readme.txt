Host Sample 05
==============
     This sample application shows building a fairly complete
     interactive console-based host which allows users to run
     cmdlets and see the output. This sample reads commands from
     the command line, executes them, and displays the results to
     the console. The sample user interface classes are built on
     top of System.Console.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating your own host interface, host user interface, and
        host raw user interface classes.
     2. Building a console application that uses these host
        classes to build an interactive PowerShell shell.
     3. Creating the $profile variable and loading the four
        different profiles:
            * Current user, current host
            * Current user, all hosts
            * All users, current host
            * All users, all hosts
     4. Implementing the IHostUISupportsMultipleChoiceSelection
        interface.
     5. Implement the IHostSupportsInteractiveSession interface
        to support interactive remoting via the Enter-PSSession
        and Exit-PSSession cmdlets.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Host05 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start command prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.

