Runspace Sample 11
==================
     This sample shows how to use the ProxyCommand class to create a proxy command that calls an
     existing cmdlet, but restricts the set of available parameters. The proxy command (a function) 
     is then added to an initial session state that is used to create a constrained runspace. The
     user can call the function, but cannot call not the initial cmdlet.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a CommandMetadata object that describes the metadata of an existing cmdlet.
     2. Modifying the cmdlet metadata to remove a parameter from the cmdlet.
     3. Adding the cmdlet to an intial session state and making it private.
     4. Creating a proxy function that calls the existing cmdlet, but exposes only a restricted
        set of parameters.
     5. Adding the proxy function to the intial session state.
     6. Calling the private cmdlet and the proxy function to demonstrate the constrained runspace.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to Runspace11 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start a Command Prompt.
     2. Navigate to the folder containing the sample executable.
     3. Run the executable.
