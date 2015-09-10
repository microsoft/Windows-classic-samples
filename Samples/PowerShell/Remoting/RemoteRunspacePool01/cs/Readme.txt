Remote Runspace Pool Sample 01
==============================
     This sample shows how to construct a remote runspace pool and how to run multiple commands
     concurrently using this pool. Although commands can be run synchronously using runspace
     pools, typically runspace pools are used to run command concurrently.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a WSManConnectionInfo object.
     2. Creating a runspace pool that uses the WSManConnectionInfo object.
     3. Running the Get-Process and Get-Service cmdlets concurrently using the remote runspace pool.
     4. Closing the runspace pool.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to the RemoteRunspacePool01 directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Verify that Windows PowerShell remoting is enabled. You can run the the following command 
        for additional information about how to enable this feature: help about_remote.
     2. Start the Command Prompt as Administrator.
     3. Navigate to the folder containing the sample executable.
     4. Run the executable.
     5. See the output results and the corresponding code.
