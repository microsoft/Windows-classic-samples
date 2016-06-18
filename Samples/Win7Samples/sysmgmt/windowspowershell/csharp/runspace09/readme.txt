Runspace Sample 09
==========================
     This sample uses the PowerShell class to execute
     a script that generates the numbers from 1 to 10 with delays
     between each number. It uses the asynchronous capabilities of
     the pipeline to manage the execution of the pipeline and
     retrieve results as soon as they are available from a
     a script.

Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to Runspace09 under the samples directory.
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

     1. Creating instances of the PowerShell class.
     2. Using these instances to execute a string as a PowerShell script.
     3. Using the PowerShell BeginInvoke method and the events on the PowerShell and
        output pipe classes to process script output asynchronously.
     4. Using the PowerShell Stop() method to interrupt an executing pipeline.
