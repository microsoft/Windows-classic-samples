select-str sample
==========================
     Searches specified files for specified patterns

Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - Visual Basic

To build the sample using Visual Studio:
=======================================
     1. Open Windows Explorer and navigate to SelectString under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The library will be built in the default \bin or \bin\Debug directory.


To run the sample:
=================
     1. Start Command Prompt.
     2. Navigate to the folder containing the sample dll.
     3. Run installutil "SelectStrCommandSample.dll"
     4. Start PowerShell
     5. Run Add-PSSnapin SelectStrPSSnapIn (this adds the PowerShell snap-in to the shell)
     6. Now type
            get-command select-str
        to verify that the command is available and to see the parameters it has.
     6. To run the select-str command, type
	    select-str -path <pathoffilestosearch> -pattern <pattern> -script <scriptblock> [-casesensitive] [-simplematch]


Demonstrates
============
     This sample demonstrates the following:
     1.Usage of PSPath
     2.Usage of Scriptblocks
     3.Session state

