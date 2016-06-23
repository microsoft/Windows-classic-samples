Runspace Sample 10
==========================
     Sample adds a cmdlet to InitialSessionState and then uses 
     modified InitialSessionState to create the Runspace.

Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to Runspace10 under the samples directory.
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

     1. Creating an initial session state object.
     2. Creating and adding a cmdlet entry to that
        initial session state.
     3. Creating a runspace from that initial session state object.
     4. Creating a pipeline
     5. Creating a command using a cmdlet that was added
        and adding it to the pipeline.
     6. Working with the PSObjects returned from the invocation    


