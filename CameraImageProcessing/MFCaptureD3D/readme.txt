MFCaptureD3D Sample
================================
Demonstrates how to preview video from a capture device, using 
Direct3D to draw the video frames.


Sample Language Implementations
===============================
C++


Files:
=============================================
BufferLock.h
Debug
device.cpp
device.h
MFCaptureD3D.h
MFCaptureD3D.rc
MFCaptureD3D.sln
MFCaptureD3D.vcproj
preview.cpp
preview.h
readme.txt
resource.h
winmain.cpp


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the MFCaptureD3D directory.
     2. Type msbuild MFCaptureD3D.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the MFCaptureD3D directory.
     2. Double-click the icon for the MFCaptureD3D.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type MFCaptureD3D.exe at the command line, or double-click the icon for MFCaptureD3D.exe to launch it from Windows Explorer.
     
On startup, the application enumerates the available video-capture 
devices and begins streaming from the first device in the list.

To select a different capture device, select Choose Device from the 
File menu.
