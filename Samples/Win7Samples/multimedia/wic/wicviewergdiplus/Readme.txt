Sample Name
WIC Viewer GDI+


Demonstrates
This code sample shows a Windows application using WIC to decode an image file and GDI+ to render the image to the screen.  

Languages
Sample Language Implementations
===============================
This sample is available in the following language implementations:
C++


Files
Resource.h: Header file that defines the identifiers for resources
WICViewerGDIPlus.h: Header file that declares application class DemoApp interface
WICViewerGDIPlus.rc: Resource file
WICViewerGDIPlus.cpp: Implementation of the application class interface
WICViewerGDIPlus.vcproj: Visual Studio 2008 project file
WICViewerGDIPlus.sln: Visual Studio 2008 solution file

Prerequisites
N/A


Building the Sample
To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild WICViewerGDIPlus.sln.

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
    
Running the Sample
After the application is launched, load an image file through file open menu. Window resizing is supported.      