Alarm Clock Sample
------------------
Demonstrates a application that sends notifications to a Windows SideShow-compatible device. The application registers with the platform and sends notifications based on user specified alarm times.
Output: The application should open a window titled 'My Alarm' and allow you to set an alarm time. When the alarm time elapses, a notification is shown on the device.

This sample demonstrates the use of the following classes and interfaces:
ISideShowSession
ISideShowNotificationManager
ISideShowNotification
ISideShowContentManager
ISideShowContent


Build Steps
-----------
To build the sample using the command prompt:
     1. Copy source files from <PSDK Install Folder>\Samples\WinUI\SideShow\Alarms
        to a working folder not under \Program Files.
     2. Open the Windows SDK CMD shell 
        (Start -> All Programs -> Windows SDK v7.0 -> SDK CMD Shell).
     3. Navigate to the directory where you copied the sample source files.
     4. Type msbuild Alarms.sln /p:platform=[win32|X64].

To build the sample using Visual Studio 2008 (preferred method):
     1. Copy source files from <PSDK Install Folder>\Samples\WinUI\SideShow\Alarms
        to a working folder not under \Program Files.
     2. Open Windows Explorer and navigate to the directory 
        where you copied the sample source files.
     3. Double-click the icon for the Alarms.sln soluion file 
        to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. 
        The sample will be built in the default \Debug or \Release directory.


Steps to run sample
-------------------
1. Register: Register the application with the Windows SideShow platform: Run the .reg registry file and select to enter the data into the registry
2. Simulator: See instructions to setup and start the simulator
3. Open the Windows SideShow control panel: Control Panel -> Hardware and Sound -> Windows SideShow
4. Enable the check box for this sample application
5. Run the sample executable
6. View the output on the simulator

Simulator Setup
---------------
Installation steps (first time only)
1. Run <PSDK Install Folder>\Bin\WindowsSideShowVirtualDevice /regserver

Starting
1. Run simulator for Windows SideShow from <PSDK Install Folder>\Bin\VirtualSideShow.exe
2. Installation required on first run


FAQ
---
Q. Why does the simulator not work on Windows XP or earlier Windows releases?
A. The Windows SideShow platform and simulator is supported only on Windows Vista.

Q. Why don’t I see the sample application on the device?
A. Make sure you register the application with the appropriate .reg file (in the sample folder). Then start the simulator and enable it through the Windows SideShow control panel. See the steps to run a sample.

Q. Why is there no data sent to the device?
A. Ensure that the application is enabled and appears on the simulator. Ensure that you run the simulator and then run the sample application.
