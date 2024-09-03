---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Power setting notification sample
urlFragment: PowerSettingRegisterNotification
description: "Shows how to register for changes in energy saver status."
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

Power Setting Register Notification (`GUID_ENERGY_SAVER_STATUS`) sample
========================================

This sample shows how to use the Power Setting Register Notification API (PowerSettingRegisterNotification) to listen to the `GUID_ENERGY_SAVER_STATUS` setting in particular.

This app demonstrates the following
- Registering for the `GUID_ENERGY_SAVER_STATUS` power setting notification.

**Note**  This sample requires Microsoft Visual Studio to build and Windows 11 to execute.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Operating system requirements
-----------------------------

- Windows 11 SDK (build 26100 or higher)
- Windows 11 (build 26100 or higher)

Build the sample
----------------

Open Windows Explorer and navigate to the directory containing the sample. Double-click the icon for the .sln (solution) file to open the file in Visual Studio. In the Build menu, select Build Solution. The application will be built in the default \\Debug or \\Release directory.

Run the sample
--------------

Navigate to the directory that contains the new executable, using the command prompt. Type "PowerSettingRegisterNotificationSample.exe" at the command line. 
The program will register for the `GUID_ENERGY_SAVER_STATUS` notification listen for the notification to be triggered. To trigger an energy saver notification, 
toggle energy saver either on the Quick Actions menu or opening Settings > Power & Battery and toggling the energy saver setting there.  

