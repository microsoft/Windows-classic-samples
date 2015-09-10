PowerShell Sample 01
==========================
     This sample uses the InitialSessionState APIs to constrain a runspace and 
     add commands and providers. This sample will concentrate on the SDK 
     mechanisms to restrict the runspace. Script alternatives to the SDK include 
     $ExecutionContext.SessionState.LanguageMode and the PSSessionConfiguration cmdlets.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Restricting the language by setting the LanguageMode property of InitialSessionState.
     2. Adding aliases to the environment using InitialSessionState's Commands property.
     3. Marking commands as private.
     4. Removing providers using InitialSessionState's Providers property.
     5. Removing commands using InitialSessionState's Commands property.
     6. Adding commands and providers.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to PowerShell01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start command prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
     4. See the output results and the corresponding code.
