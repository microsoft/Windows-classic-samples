RMFILTERAPP Sample
===============================
The RMFILTERAPP sample is a generic Win32-based console Restart Manager
conductor application.

This sample shows the minimal amount of code your Restart Manager
conductor application should use during a servicing transaction.  
	
This sample shows how a primary installer can use Restart Manager to 
shutdown and restart a process. 

This sample uses Calculator as the target process to be shutdown and
restarted. The sample assumes that at least one instance of Calculator 
is running before starting the Restart Manager sample.

This sample requires the OS version to be Windows 7 or above.

Sample Language Implementations
===============================
C++


Files:
===============================
rmfilterapp.sln - The solution file
rmfilterapp.vcproj - The project file
rmfilterapp.cpp - The .CPP source file


To build the sample using the SDK CMD Shell:
=============================================
     1. Open the Command Prompt window and navigate to the RmFilterApp directory.
     2. Build a .sln file by typing: 
	msbuild rmfilterapp.sln

Please note if you are running on Windows X64 platform, please build the 
sample solution file for X64 platform (the default is win32) for the 
sample code to work properly. Here is the command line for building 
X64 platform in SDK CMD shell:
	msbuild rmfilterapp.sln /p:platform=X64

To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the RmFilterApp directory.
     2. Double-click the icon for the rmfilterapp.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type rmfilterapp.exe at the command line, or double-click the icon for rmfilterapp.exe to launch it from Windows Explorer.
