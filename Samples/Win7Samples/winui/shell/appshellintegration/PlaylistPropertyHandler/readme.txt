Playlist Property Handler Sample
================================
Demonstrates the implementation of a property handler for the .WPL and .ZPL playlist file formats. 

Sample Language Implementations
===============================
C++

Files:
=============================================
Dll.cpp 
Dll.h 
Exports.def 
PlaylistPropertyHandler.cpp 
PlaylistPropertyHandler.sln 
PlaylistPropertyHandler.vcproj 
Sample1.WPL 
Sample1.zpl 
Sample2.WPL 
Sample2.zpl 
Sample3.WPL 
Sample3.zpl 


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the PlaylistPropertyHandler directory.
     2. Type msbuild PlaylistPropertyHandler.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the PlaylistPropertyHandler directory.
     2. Double-click the icon for the PlaylistPropertyHandler.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new dll using the command prompt.
     2. Type regsvr32.exe PlaylistPropertyHandler.dll
     3. New sample files can be created by using New >  DocFile File  or New >  Open Metadata File 

To remove the sample:
=================
     1. Navigate to the directory that contains the dll using the command prompt.
     2. Type regsvr32.exe /u PlaylistPropertyHandler.dll
