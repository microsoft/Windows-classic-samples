Data deduplication backup and restore sample
============================================

This sample demonstrates the use of the [Data Deduplication Backup/Restore API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh449211) to perform optimized backup and optimized restore that is not optimized.

For more information about this scenario, see [Selective File Restore Using Data Deduplication](http://msdn.microsoft.com/en-us/library/windows/desktop/hh769317).

This sample is written in C++ and requires some experience with COM.

This sample contains the following files:

-   DedupBackupRestore.cpp
-   DedupBackupRestore.sln
-   DedupBackupRestore.vcxproj

**Warning**  This sample requires Windows Server 2012 R2 and Microsoft Visual Studio 2013. It will not compile in Microsoft Visual Studio Express 2013 for Windows.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related technologies
--------------------

[Data Deduplication API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh449204)

Operating system requirements
-----------------------------

Client

None supported

Server

Windows Server 2012 R2

Build the sample
----------------

To build the sample using the command line:

-   Open the **Command Prompt** window and navigate to the directory.
-   Type **msbuild DedupBackupRestore.sln**.

To build the sample using Visual Studio (preferred method):

-   Open File Explorer and navigate to the directory.
-   Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
-   In the **Build** menu, select **Build Solution**. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

To run the sample:

1.  Install the Data Deduplication component.
2.  Set up a Data Deduplication-enabled volume with some test files, and run a Deduplication Optimization job to optimize the files first.
    **Note**  The following sequence of PowerShell cmdlets assumes that your test volume is T:.
    1.  **Enable-DedupVolume -volume T:**
    2.  **Set-DedupVolume -volume T: -MinimumFileAgeDays 0**
    3.  **Start-DedupJob -volume T: -type Optimization -wait**

    For more information, see the [Enable-DedupVolume](http://technet.microsoft.com/library/4a752894-524d-4a64-8483-f06a73ab0ed0) PowerShell cmdlet documentation.
3.  Navigate to the directory that contains the new executable file, using the **Command Prompt** or **File Explorer**.
    **Note**  If you use the Command Prompt, you must run as administrator.
4.  Type the name of the executable file (DedupBackupRestore.exe by default) at the command prompt.

