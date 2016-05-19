NotificationIcon Sample
===============================
Demonstrates how to use the Shell_NotifyIcon and Shell_NotifyIconGetRect APIs to display a notification
icon. This also demonstrates how to display a rich flyout window, context menu, and balloon notifications.

Please note that Shell_NotifyIconGetRect and CalcualtePopupWindowPosition are only available on Windows 7 and higher.

Sample Language Implementations
===============================
C++

Files:
===============================
NotificationIcon.sln
NotificationIcon.vcproj
NotificationIcon.cpp
Resource.h
NotificationIcon.rc

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the NotificationIcon directory.
     2. Type msbuild NotificationIcon.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the NotificationIcon directory.
     2. Double-click the icon for the NotificationIcon.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type NotificationIcon.exe at the command line, or double-click the icon for NotificationIcon.exe to launch it from Windows Explorer.

Troubleshooting:
=================
Please note that notification icons specified with a GUID are protected against spoofing by validating
that only a single application registers them. This registration is performed the first time you 
call Shell_NotifyIcon(NIM_ADD, ...), and the full pathname of the calling application is stored. If you
later move your binary to a different location, the system will not allow the icon to be added again.

Please see the MSDN documentation for Shell_NotifyIcon for more information.
