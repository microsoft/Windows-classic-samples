SUMMARY
=======
This sample demonstrate the use of Wininet cache APIs to enumerate cache entries for different urls.


Security Note 
=============

 This sample is provided for educational purpose only to demonstrate how to use Windows 
 WinHTTP API. It is not intended to be used without modifications in a production 
 environment and it has not been tested in a production environment. Microsoft assumes 
 no liability for incidental or consequential damages should the sample code be used for 
 purposes other than as intended.

USAGE
=====
This sample includes Microsoft Visual Studio .NET project files. To create CacheEnumerate.exe,
load CacheEnumerate.sln and build the project.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild CacheEnumerate.sln.


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================
Usage: CacheEnumerate [-?] | [[-a | -c | -h | -k] -d] | [-e <URL>]
    Flag Semantics:
    -a : Enumerate entries in all fixed containers
    -c : Enumerate entries in the content container
    -d : Delete entries option
    -e : Get details of a entry for a specific URL
    -h : Enumerate entries in the history container
    -k : Enumerate entries in the cookie container
    -? : Display usage info.
For example:
     CacheEnumerate.exe -a - Enumerate all entries in all the fixed containers
     CacheEnumerate.exe -e http://www.microsoft.com/ - get detail on an entry associated a URL
     CacheEnumerate.exe -h -d - Enumerate all the entries in the history container and delete each entry found
     CacheEnumerate.exe -a -d - Enumerate all entries in the fixed containers

SOURCE FILES
=============
CacheEnumerate.h
CacheEnumerate.cpp
CacheEnumerate.vcproj
CacheEnumerate.sln

SEE ALSO
=========
For more information on Wininet caching, go to:
http://msdn.microsoft.com/en-us/library/aa383928.aspx


==================================
© Microsoft Corporation
