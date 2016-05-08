Image Factory API Sample
========================
This sample demonstrates how to use IShellItemImageFactory to get the best possible image for an item.
It uses IShellItem to factory IShellItemImageFactory interface. From IShellItemImageFactory, it gets the
best possible image for an item


Sample Language Implementations
===============================
C++

Files
=============================================
ImageFactorySample.cpp
ImageFactorySample.rc
ImageFactorySample.sln
ImageFactorySample.vcproj
resource.h

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild ImageFactorySample.sln


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the sample directory.
     2. Double-click the icon for the ImageFactory.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable using the command prompt.
     2. ImageFactorySample.exe <size> <Absolute Path to file>
