Select-Object Sample 01
=======================
     This sample creates a cmdlet called Select-Obj which acts as a filter to select only certain
     objects to process or pass down the pipeline. It is most effectively used as a pipeline
     receiver from other cmdlets such as Get-Service or Get-Process. The First, Last, and 
     Unique parameters select which objects to process. The cmdlet works with files, modules,
     registry keys, and other objects.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating an advanced cmdlet.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open File Explorer and navigate to SelectObject01 under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution.
     4. The library will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Store the assembly in the following module folder:
        [user]/Documents/WindowsPowerShell/Modules/SelectObjSample01
     2. Start Windows PowerShell.
     3. Run the following command: Import-Module SelectObjSample01
        (This command loads the assembly into Windows PowerShell.)
     4. Type the following command to run the cmdlet: Select-Obj
