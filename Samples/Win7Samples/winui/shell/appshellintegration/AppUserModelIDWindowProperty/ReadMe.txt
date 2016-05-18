AppUserModelID Window Property Sample
================================
Demonstrates how to control the taskbar grouping behavior of an application's windows by setting the AppUserModelID on a window via the use of the IPropertyStore implementation for windows, obtained via SHGetPropertyStoreForWindow.

Sample Language Implementations
===============================
C++

Files:
=============================================
AppUserModelIDWindowProperty.cpp
AppUserModelIDWindowProperty.ico
AppUserModelIDWindowProperty.rc
resource.h


Prerequisites:
=============================================
     None


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the AppUserModelIDWindowProperty directory.
     2. Type msbuild AppUserModelIDWindowProperty.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the AppUserModelIDWindowProperty directory.
     2. Double-click the icon for the AppUserModelIDWindowProperty.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type AppUserModelIDWindowProperty.exe at the command line, or double-click the icon for AppUserModelIDWindowProperty.exe to launch it from Windows Explorer.
     3. To demonstrate the effect AppUserModelIDs have on taskbar grouping, launch at least 3 instances of the application at the same time.
     4. Use the menu to set a different AppUserModelID on each of the three windows.  Notice that each AppUserModelID results in a separate taskbar button, and that windows can change their identity at runtime.
     5. Set at least two windows to the second AppUserModelID.  Notice that they both move into the same taskbar group.
     6. Set the taskbar to "Combine when taskbar is full" or "Never combine" to see how each window can get a separate button, but that the buttons are grouped by AppUserModelID.
