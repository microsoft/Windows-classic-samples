Job Source Adapter Sample
=========================
     This sample shows how to derive a FileCopyJob class from the Job2 type and a FileCopyJobSourceAdapter
     class from the JobSourceAdapter type. The FileCopyJob sample class is implemented to perform simple
     file system listening and file copying functions. The FileCopyJobSourceAdapter implementation creates
     FileCopyJob objects and allows manipulation of these objects through Windows PowerShell's Get-Job,
     Suspend-Job, Resume-Job, Stop-Job, and Remove-Job cmdlets.

     For Windows PowerShell information on MSDN, see http://go.microsoft.com/fwlink/?LinkID=178145


Sample Objectives
=================
     This sample demonstrates the following:

     1. Creating a FileCopyJob job derived from a System.Management.Automation.Job2 type.
     2. Creating a FileCopyJobSourceAdapter derived from a System.Management.Automation.JobSourceAdapter type.
     3. Importing the assembly and the FileCopyJobSourceAdapter into a PowerShell console so that existing
        PowerShell job cmdlets can be used to manipulate FileCopyJob job objects.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:

     - C#


Building the Sample Using Visual Studio
=======================================
     1. Open Windows Explorer and navigate to the JobSourceAdapter directory under the samples directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. Make sure the JobSourceAdapter project references the System.Management.Automation.dll assembly.
     4. In the Build menu, select Build Solution.
     5. The executable will be built in the default \bin or \bin\Debug directory.


Running the Sample
==================
     1. Run Import-Module with the full path to the sample DLL.
     2. Run the Get-FileCopyJob cmdlet that was imported from the assembly. Create one or more 
        FileCopyJob objects passing in the name, text source file, and text destination file paths.
     3. Use Get-Job to see the FileCopyJob jobs that were created.
     4. Use Suspend-Job and Resume-Job to suspend and resume the jobs.
     5. Use Stop-Job and Remove-Job to stop and remove the jobs from the JobSourceAdapter repository.

