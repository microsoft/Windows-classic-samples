Sample Name
WIC Progressive Encoding


Demonstrates
This code sample shows a Windows application using WIC to decode an image that is encoded with progressive levels. 
It uses D2D as rendering API to render frames at the different  progressive level to the screen.  

Languages
Sample Language Implementations
===============================
This sample is available in the following language implementations:
C++


Files
Resource.h: Header file that defines the identifiers for resources
WICProgressiveDecoding.h: Header file that declares application class DemoApp interface
WICProgressiveDecoding.rc: Resource file
WICProgressiveDecoding.cpp: Implementation of the application class interface
WICProgressiveDecoding.vcproj: Visual Studio 2008 project file
WICProgressiveDecoding.sln: Visual Studio solution file

Prerequisites
N/A


Building the Sample
To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild WICProgressiveDecoding.sln.

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.
     
     
Running the Sample
After the application is launched, load an image file through file open menu. On loading, the default progressive level is set to 0. The user can navigate to different 
progressive levels either through menu or Up/Down key. The current progressive level text is displayed on the main window status bar. Window resizing is supported.