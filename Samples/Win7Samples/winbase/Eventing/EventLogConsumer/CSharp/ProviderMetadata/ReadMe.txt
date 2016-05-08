ProviderMetadata Sample


The ProviderMetadata sample demonstrates some of the provider and event discovery capabilities of Windows Event Log.  
This sample enumerates installed providers and reads some of the provider and event properties.


Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C#

Files
=============================================
    ReadMe.txt
    Metadata.cs
    ProviderMetadata.csproj
    ProviderMetadata.sln 
 
Prerequisites
=============================================
    1. Microsoft Windows Vista or later
    2. Microsoft .NET3.5 or later
    3. Visual Studio 2008 (for ChannelConfig.sln solution file)
 

To build the sample using the command prompt:
=============================================
     1. Open Microsoft Windows SDK Command Prompt window and navigate to the sample directory.
     2. Type msbuild ProviderMetadata.csproj.


To build the sample using Visual Studio 2008:
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Type ProviderMetadata.exe at the command line.


Additional Run Steps:
=====================
     1. Run ProviderMetadata.exe /? for further instructions.


