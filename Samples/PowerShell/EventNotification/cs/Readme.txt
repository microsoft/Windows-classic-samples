Event Notification Sample
=========================
     This sample shows how to receive notifications of Windows PowerShell events that are 
     generated on remote computers. It uses the PSEventReceived event exposed through the 
     Runspace class.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145 


Sample Objectives
=================
     This sample demonstrates the following:

     1. How to use the PSEventReceived event to receive notification of Windows PowerShell events that are 
        generated on remote computers.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to the Events02 directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Verify that Windows PowerShell remoting is enabled; you can run the the following command 
        for additional information about how to enable this feature: help about_remote.
     2. Start the command prompt.
     3. Navigate to the folder containing the sample executable.
     4. Run the executable.
     5. See the output results and the corresponding code.
