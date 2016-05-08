Known Folders
=============================================
Demonstrates the most common developer uses of the new KnownFolder API: defining, registering, enumerating and finding the path for all KnownFolders on your system.  

A sample registry file is also provided to demonstrate registering a KnownFolder by directly writing the relevant registry keys and values. This sample registry file showcases only the most common KnownFolder definition fields and targets developers of managed code who would prefer registry access to COM interop.

Sample Language Implementations
=============================================
C++

Files:
=============================================
kfexplorer.cpp
kfexplorer.rc
resource.h
KnownFolders.sln
KnownFolders.vcproj
kfdef.reg

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the KnownFolders directory.
     2. Type msbuild KnownFolders.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the KnownFolders directory.
     2. Double-click the icon for the KnownFolders.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. Run KnownFolders.exe
