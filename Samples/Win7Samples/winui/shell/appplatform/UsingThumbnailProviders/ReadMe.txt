Image Factory API Sample
========================
This sample demonstrates the use of IThumbnailProvider API. IThumbnailProvider is the interface Windows 
thumbnail cache system uses to extract the thumbnail for an item. It is given higher priority over IExtractImage
interface.


Sample Language Implementations
===============================
C++

Files
=============================================
ThumbnailProvider.cpp
ThumbnailProvider.sln
ThumbnailProvider.vcproj

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild ThumbnailProvider.sln


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the sample directory.
     2. Double-click the icon for the ThumbnailProvider.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. ThumbnailProvider.exe <size> <Absolute Path to file>
The first parameter is the size of the thumbnail to retrieve. The second parameter is the full path of the file for which
the thumbnails needs to be retrieved.
