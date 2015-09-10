Runspace Sample 06
==================
     This sample shows how to add a module to an initial session state, how to create a runspace
     that uses the initial session state, and how to run a cmdlet that is provided by the module.

     This sample assumes that the user has the GetProcessSample02.dll that is produced by the
     GetProcessSample02 sample copied to the current directory.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a default initial session state.
     2. Creating a runspace that uses the initial session state.
     3. Creating a PowerShell object that uses the runspace.
     4. Adding a cmdlet of a module to the PowerShell object.
     5. Using PSObject objects to extract and display properties from 
        the objects returned by the cmdlet.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#

Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace06 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Build the GetProcessSample02 sample.
     2. Start a Command Prompt.
     3. Navigate to the folder containing the Runspace06 sample executable.
     4. Copy GetProcessSample02.dll to the current directory.
     5. Run the executable.
