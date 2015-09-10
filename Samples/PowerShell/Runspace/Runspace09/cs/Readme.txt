Runspace Sample 09
==================
     This sample shows how to use a PowerShell object to run a script that generates the numbers
     from 1 to 10 with delays between each number. The pipeline of the PowerShell object is run
     asynchronously and events are used to handle the output.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a PowerShell object.
     2. Adding a script to the pipeline of the PowerShell object.
     3. Using the BeginInvoke method to run the pipeline asynchronosly.
     4. Using the events of the PowerShell object to process the output of the script.
     5. Using the PowerShell.Stop() method to interrupt an executing pipeline.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace09 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
