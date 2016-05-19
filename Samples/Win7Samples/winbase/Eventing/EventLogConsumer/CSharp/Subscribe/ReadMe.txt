Subscribe sample


The Subscribe sample demonstrates some of the subscription capabilities of Windows Event Log.  
This sample allows the user to subscribe to a given channel with an optional query
filter and displays the events written to the log inside a callback method.    


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C#

Files
=============================================
    ReadMe.txt
    Subscribe.cs
    StructuredQuery.xml
    Subscribe.csproj
    Subscribe.sln 
 
Prerequisites
=============================================
    1. Microsoft Windows Vista or later
    2. Microsoft .NET3.5 or later
    3. Visual Studio 2008 (for Subscribe.sln solution file)
 


To build the sample using the command prompt:
=============================================
     1. Open Microsoft Windows SDK Command Prompt window and navigate to the sample directory.
     2. Type msbuild Subscribe.csproj.


To build the sample using Visual Studio 2008:
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Type "Subscribe.exe APPLICATION" at the command line. The application will wait for events to arrive to the Application log.
     3. To create an event into the Application channel, launch another command prompt and type the following command to:
           EVENTCREATE /T INFORMATION /ID 123 /L APPLICATION /D "Test"
     


Additional Run Steps:
=====================
     1. Run Subscribe.exe /? for further instructions.


