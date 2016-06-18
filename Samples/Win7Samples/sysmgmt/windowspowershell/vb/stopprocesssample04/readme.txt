stop-process Sample 04
==========================
     Stops a specified process


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - Visual Basic


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to StopProcessSample04 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The library will be built in the default \bin or \bin\Debug directory.


To run the sample:
=================
     1. Start Command Prompt.
     2. Navigate to the folder containing the sample dll.
     3. Run installutil "StopProcessSample04.dll"
     4. Start PowerShell
     5. Run Add-PSSnapin StopProcPSSnapIn04 (this adds the PowerShell snap-in to the shell)
     6. Now type "stop-proc <procname>" to run the Cmdlet that this sample contains.

  

Demonstrates
============
     This sample demonstrates the following:
     
     1.Creating a basic Cmdlet
     2.Adding parameter declaration
     3.Specifying positions for parameters
     4.Taking input from the pipeline and taking input from the pipeline through property name
     5.Error Handling and Exceptions
     6.Using ShouldProcess and ShouldContinue methods
     7.Using Force and PassThru parameters
     8.User notifications using WriteWarning, WriteVerbose and WriteDebug
     9.Aliases, wildcards and help message
    10.Using InputObject
    11.Deriving from PSCmdlet
	



