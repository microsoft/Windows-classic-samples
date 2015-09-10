Runspace Sample 10
==================
     This sample shows how to add a cmdlet to an InitialSessionState object and then use the
     modified InitialSessionState object when creating a Runspace object.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating an InitialSessionState object.
     2. Adding a cmdlet to the InitialSessionState object.
     3. Creating a runspace that uses the InitialSessionState object.
     4. Creating a PowerShell object that uses the Runspace object.
     5. Running the pipeline of the PowerShell object synchronously.
     6. Working with PSObject objects to extract properties from the objects returned by the pipeline.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace10 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
