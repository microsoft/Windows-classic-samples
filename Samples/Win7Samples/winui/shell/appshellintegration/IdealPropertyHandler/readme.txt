Ideal Property Handler Sample
================================
Demonstrates the implementation of an ideal property handler for an OleDoc File Format (.docfile-ms) or a fictional OpenMetadata format (.openmetadata-ms) which supports reading and writing properties and custom schema.


Sample Language Implementations
===============================
C++

Files:
=============================================
Sample.openmetadata-ms
Dll.cpp
dll.h
DocFile.ico
DocFileHandler.cpp
OpenMetadata.ico
OpenMetadataHandler.cpp
PropertyHandler.def
PropertyHandler.rc
PropertyHandler.sln
PropertyHandler.vcproj
resource.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the IdealPropertyHandler directory.
     2. Type msbuild PropertyHandler.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the IdealPropertyHandler directory.
     2. Double-click the icon for the PropertyHandler.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new dll using the command prompt.
     2. Type regsvr32.exe PropertyHandler.dll
     3. New sample files can be created by using New > "DocFile File" or New > "Open Metadata File"

To remove the sample:
=================
     1. Navigate to the directory that contains the dll using the command prompt.
     2. Type regsvr32.exe /u PropertyHandler.dll
