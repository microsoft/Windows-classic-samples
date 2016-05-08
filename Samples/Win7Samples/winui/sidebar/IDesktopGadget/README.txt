IDesktopGadget Sample
================================
Demonstrates how to use the IDesktopGadget interface to programmatically add Windows desktop gadgets to the current user's desktop.

The gadget path specified must be under one of the following three locations:
1. The Windows Desktop Gadget path (%ProgramFiles%\Windows Sidebar\Gadgets)
2. The shared Desktop Gadget path (%ProgramFiles%\Windows Sidebar\Shared Gadgets)
3. The user's Desktop Gadget path (%LocalAppData%\Microsoft\Windows Sidebar\Gadgets)

Sample Language Implementations
===============================
C++

Files:
=============================================
main.cpp

Prerequisites:
=============================================
<none>


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the sample's directory.
2. Type msbuild IDesktopGadget.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
1. Open Windows Explorer and navigate to the sample's directory.
2. Double-click the icon for the IDesktopGadget.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new executable, using the command prompt.
2. Type IDesktopGadget.exe at the command line for usage information.


