get-process Sample 03
==========================
     This sample shows how to create a cmdlet that can take input from pipeline. 


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to GetProcessSample03 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
        The library will be built in the default \bin or \bin\Debug directory.


To run the sample:
=================
     1. Store the assembly in the following module folder:
        [user]/documents/windowspowershell/modules/GetProcessSample03 
     2. Start Windows PowerShell.
     3. Run the following command: import-module GetProcessSample03
        (This command loads the assembly into Windows PowerShell.
     4. Now type the following command to run the cmdlet: get-proc

  

Demonstrates
============
     This sample demonstrates the following:
     
     1. Declaring a cmdlet class.
     2. Declaring a parameter.
     3. Specifying the position of the parameter.
     4. Specifying that the parameter takes input from the pipeline. The input can be taken 
        from an obect, or taken from a property of an object that has the same name as 
        the parameter.
	



