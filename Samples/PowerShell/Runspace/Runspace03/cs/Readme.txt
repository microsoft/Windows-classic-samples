Runspace Sample 03
==================
     This sample uses the PowerShell class to execute a script that retrieves process information
     for the list of process names passed into the script. It shows how to pass input objects to
     a script and how to retrieve error objects as well as the output objects.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating an instance of the PowerShell class.
     2. Using this instance to execute a string as a PowerShell script.
     3. Passing input objects to the script from the calling program.
     4. Using PSObject to extract and display properties from the objects returned by this command.
     5. Retrieving and displaying error records generated during execution of the script.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#

Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace03 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
