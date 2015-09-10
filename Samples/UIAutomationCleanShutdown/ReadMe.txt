================================
UIA Clean Shutdown Sample
Microsoft UI Automation
================================
This sample is two pieces that work together to show proper cleanup/shutdown of a UIA control hosted in a DLL.

The first part is a DLL containing a simple control that has a basic UIA provider. This provider makes sure to call
the UiaDisconnectProvider API when the control is destroyed. This ensures that all external UIA clients release
their references to this provider, so the dll can unload safely.

The second part is a host application that can host the control. When it starts up it simply has a Load DLL button.
When this is pressed it loads the dll and uses it to create the control. It then changes to an Unload DLL button.
When this is pressed it destroys the control and unloads the dll.

If a UIA client has connected during this time this will free the references so the dll can safely unload.

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
UiaCleanShutdownControl.cpp		DllMain and control source file
UiaCleanShutdownControl.def		Contains the list of exported functions from the dll
UiaCleanShutdownControl.h		Header for the control
UiaCleanShutdownControl.vcxproj		VS project for the control dll
UiaProvider.cpp				UI Automation provider implementation for the control
UiaProvider.h				Header for the UI Automation implementation
UiaCleanShutdownHost.cpp		Source file for the host window
UiaCleanShutdownHost.exe.manifest	Manifest for the host application (enables common control v6)
UiaCleanShutdownHost.rc			Resource file for the host application
UiaCleanShutdownHost.vcxproj		VS project for the host application
UiaCleanShutdown.sln			VS solution for both the dll and application
ReadMe.txt				This readme

========
Building
========

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild UiaCleanShutdown.sln


To build the sample using Visual Studio 2011 (preferred method):
================================================
     1. Open File Explorer and navigate to the  directory.
     2. Double-click the icon for the UiaCleanShutdown.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application  and dll will be built in the default \Debug or \Release directory.


=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable and dll, using the command prompt or File Explorer.
     2. Type UiaCleanShutdownHost.exe at the command line, or double-click the icon for UiaCleanShutdownHost.exe 
	to launch it from File Explorer.

To run from Visual Studio, press F5 or click Debug->Start Debugging.

========
Comments
========
While running, the best way to see it working it to hit the "Load DLL" button and then point a UIA client at it. Two good clients
are the built in screen reader Narrator and the SDK tool Inspect which is included in the Windows SDK.