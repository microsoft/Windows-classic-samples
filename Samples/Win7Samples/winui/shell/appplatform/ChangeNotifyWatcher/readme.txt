Change Notify Watcher Sample
================================
This sample demonstrates how to listen to Shell Change Notifications on a folder or item in the Explorer 
namespace.  

Sample Language Implementations
===============================
C++

Files
=============================================
ChangeNotifyWatcher.cpp
ChangeNotifyWatcher.sln
ChangeNotifyWatcher.vcproj
DragDropHelpers.h
LogWindow.h
ResizeableDialog.h
resource.h
resource.rc
ShellHelpers.h


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the ChangeNotifyWatcher directory.
     2. Type msbuild ChangeNotifyWatcher.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the ChangeNotifyWatcher directory.
     2. Double-click the icon for the ChangeNotifyWather.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type ChangeNotifyWatcher.exe at the command line, or double-click the icon for ChangeNotifyWatcher.exe to launch it from Windows Explorer.
     3. Select a folder to watch by either clicking "Pick..." or by drag & dropping a folder from a Windows Explorer window into the sample's listview.
