stop-process Sample 01
==========================
     This sample shows how to write a cmdlet that implements a PassThru parameter that 
     indicates that the user wants the cmdlet to return an object, and how to request 
     user feedback by calls to the ShouldProcess and ShouldContinue methods. This cmdlet 
     stops a specified process.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to StopProcessSample01 under 
        the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file 
        in Visual Studio.
     3. In the Build menu, select Build Solution.
        The library will be built in the default \bin or \bin\Debug directory.


To run the sample:
=================
     1. Store the assembly in the following module folder:
        [user]/documents/windowspowershell/modules/StopProcessSample01 
     2. Start Windows PowerShell.
     3. Run the following command: import-module StopProcessSample01
        (This command loads the assembly into Windows PowerShell.)
     4. Now type the following command to run the cmdlet: stop-proc
  

Demonstrates
============
     This sample demonstrates the following:
     
     1. Declaring a cmdlet class.
     2. Declaring cmdlet parameters.
     3. Specifying positions for parameters.
     4. Specifying that the parameters can accept an object from the pipeline 
        or accept a value from a property of an object that has the same name 
        as the parameter.
     5. Error handling and exceptions.
     6. Using the ShouldProcess and ShouldContinue methods.
     7. Implementing the Force and PassThru parameters.
	



