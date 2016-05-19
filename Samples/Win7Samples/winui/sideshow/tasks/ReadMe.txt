Task List Sample
----------------
Uses the events and the Simple Content Format more extensively. The sample application gets its data from the TaskList.xml file in the sample folder. It uses the ContentMissing event to send content to the device.

Output: A page of tasks is sent to the device and can be found under the 'Tasks Sample' application on the device. The device will contain a list of tasks in a menu page and selecting any one of those items will cause content for the specific task to be retrieved from the sample application.

This sample demonstrates the use of the following classes and interfaces:
ISideShowSession
ISideShowContentManager
ISideShowContent
ISideShowEvents


Build Steps
-----------
This sample cannot be built in the SDK environment.  Visual Studio must be used.
To build the sample using Visual Studio 2008:
     1. Copy source files from <PSDK Install Folder>\Samples\WinUI\SideShow\Tasks
        to a working folder not under \Program Files.
     2. Open Windows Explorer and navigate to the directory 
        where you copied the sample source files.
     3. Double-click the icon for the Tasks.sln soluion file 
        to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. 
        The sample will be built in the default \Debug or \Release directory.


Steps to run sample
-------------------
1. Register: Register the application with the Windows SideShow platform: regsvr32 WindowsSideShowTasks.dll
   (requires administrator privileges)
2. Simulator: See instructions to setup and start the simulator
3. Open the Windows SideShow control panel: Control Panel -> Hardware and Sound -> Windows SideShow
4. Enable the check box for this sample application.  The sample gadget will start running automatically.
5. View the output on the simulator

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

Q. Why don't I see the sample application on the device?
A. Make sure you register the application with the appropriate .reg file (in the sample folder). Then start the simulator and enable it through the Windows SideShow control panel. See the steps to run a sample.

Q. Why is there no data sent to the device?
A. Ensure that the application is enabled and appears on the simulator. Ensure that you run the simulator and then run the sample application.
