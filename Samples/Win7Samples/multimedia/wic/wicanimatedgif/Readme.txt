Sample Name
WIC Animated Gif


Demonstrates
WIC added support for reading/writing GIF metadata which is essential for rendering Animated GIFs. 
This code sample is a simple Windows application that demonstrates decoding various frames in an GIF file, reading appropriate metadata for each frame, composing frames, and rendering the animation with Direct2D.


Languages
Sample Language Implementations
===============================
This sample is available in the following language implementations:
        C++


Files
WICAnimatedGif.vcproj
WICAnimatedGif.sln
sample.gif: sample gif file
Resource.h: Header file that defines the identifiers for resources
WICAnimatedGif.h: Header file that declares application class DemoApp interface
WICAnimatedGif.rc: Resource file
WICAnimatedGif.cpp: Implementation of the application class interface
DeclareDPIAware.manifest: manifest file to declare DPI aware
Readme.txt


Prerequisites
Windows Software Development Kit (SDK) for Windows 7


Building the Sample
To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild WICAnimatedGif.sln.

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.