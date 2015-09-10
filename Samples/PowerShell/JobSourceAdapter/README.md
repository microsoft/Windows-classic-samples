Windows PowerShell Job Source Adapter Sample
============================================

This sample shows how to derive a **FileCopyJob** class from the [**Job2**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485406) type and a **FileCopyJobSourceAdapter** class from the [**JobSourceAdapter**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485414) type.

The **FileCopyJob** sample class is implemented to perform simple file system listening and file copying functions. The **FileCopyJobSourceAdapter** implementation creates **FileCopyJob** objects and allows manipulation of these objects through Windows PowerShell's Get-Job, Suspend-Job, Resume-Job, Stop-Job, and Remove-Job cmdlets.

**Sample Objectives**

This sample demonstrates the following:

1.  Creating a **FileCopyJob** job derived from a [**Job2**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485406) type.
2.  Creating a **FileCopyJobSourceAdapter** derived from a [**JobSourceAdapter**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485414) type.
3.  Importing the assembly and the **FileCopyJobSourceAdapter** into a PowerShell console so that existing PowerShell job cmdlets can be used to manipulate **FileCopyJob** job objects.

Related topics
--------------

**Conceptual**

[Windows PowerShell](http://go.microsoft.com/fwlink/p/?linkid=178145)

**Reference**

[**Job2**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485406)

[**JobSourceAdapter**](http://msdn.microsoft.com/en-us/library/windows/desktop/hh485414)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Microsoft Visual Studio and select **File** \> **Open** \> **Project/Solution**.
2.  Go to the directory named for the sample, and double-click the Visual Studio Solution (.sln) file.
3.  Make sure the JobSourceAdapter project references the System.Management.Automation.dll assembly.
4.  Press F7 or use **Build** \> **Build Solution** to build the sample.

    The executable will be built in the default **\\bin** or **\\bin\\Debug** directory.

Run the sample
--------------

1.  Run `Import-Module` with the full path to the sample DLL.
2.  Run the `Get-FileCopyJob` cmdlet that was imported from the assembly. Create one or more **FileCopyJob** objects passing in the name, text source file, and text destination file paths.
3.  Use `Get-Job` to see the FileCopyJob jobs that were created.
4.  Use `Suspend-Job` and `Resume-Job` to suspend and resume the jobs.
5.  Use `Stop-Job` and `Remove-Job` to stop and remove the jobs from the JobSourceAdapter repository.

