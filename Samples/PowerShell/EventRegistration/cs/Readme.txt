Event Registration Sample
=========================
    This sample shows how to create a cmdlet for event registration by deriving
    from ObjectEventRegistrationBase.

    The sample creates the Register-FileSystemEvent cmdlet which subscribes to
    events raised by System.IO.FileSystemWatcher. With this cmdlet, users can
    register an action to execute when a specific event is raised, such as
    when a file is created under an specific directory.

    For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145 


Sample Objectives
=================
     This sample demonstrates the following:

     1. How to how to derive from the ObjectEventRegistrationBase class to create a cmdlet for event registration.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to the Events01 directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library file will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Start PowerShell and import the library file in order to make the Register-FileSystemEvent 
        cmdlet available in PowerShell.
     2. Use the Register-FileSystemEvent cmdlet to register an action that will write a message
        when a file is created under the TEMP directory.
     3. Create a file under the TEMP directory and note that the action is executed (i.e. the
        message is displayed).
        
     This is the sample output from executing these 3 steps:
     
        PS> Import-Module .\bin\Debug\Events01.dll
        PS> Register-FileSystemEvent $env:temp Created -filter "*.txt" -action { Write-Host "A file was created in the TEMP directory" }

        Id  Name            State      HasMoreData  Location  Command
        --  ----            -----      -----------  --------  -------
        1   26932870-d3b... NotStarted False                  Write-Host "A f...

        PS> Set-Content $env:temp\test.txt "This is a test file"
        A file was created in the TEMP directory
        PS>
