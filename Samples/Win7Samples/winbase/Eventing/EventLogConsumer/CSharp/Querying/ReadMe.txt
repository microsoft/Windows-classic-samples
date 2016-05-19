ReadEvents sample


The ReadEvents sample demonstrates the use of querying and rendering capabilities of Windows Event Log.  
This sample queries a log or log file and displays the selected events in the Xml format fully 
or just the provider name and description.
     
This sample has similar command-line syntax as wevtutil.exe utility,
and implements a subset of wevtutil.exe functionality.
    

Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C#

Files
=============================================
    ReadMe.txt
    ReadEvents.cs
    ReadEvents.csproj
    ReadEvents.sln 
 
Prerequisites
=============================================
    1. Microsoft Windows Vista or later
    2. Microsoft .NET3.5 or later
    3. Visual Studio 2008 (for ReadEvents.sln solution file)


To build the sample using the command prompt:
=============================================
     1. Open Microsoft Windows SDK Command Prompt window and navigate to the sample directory.
     2. Type msbuild ReadEvents.csproj.


To build the sample using Visual Studio 2008:
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     NOTE: you may need to run this code with administrator privileges to be able to view events stored in certain logs,
           depending on the log's security settings.

     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Type "ReadEvents.exe System /count:1" at the command line.


Additional Run Steps:
=====================
     1. Run ReadEvents.exe /? for further instructions.


