Pictures Sample
---------------
Demonstrates the sending of pictures / images from a file to an Windows SideShow device. The application takes jpeg images from the Pictures directory and sends them to the device to be viewed.

Output: The 'Pictures Sample' application on the device is sent a set of pictures. The first picture is shown on entering the application on the device. The different images can be viewed by navigating with the right and left buttons on the device or simulator.

This sample demonstrates the use of the following classes and interfaces:
ISideShowSession
ISideShowContentManager
ISideShowContent
ISideShowEvents


Build Steps
-----------
To build the sample using the command prompt:
     1. Copy source files from <PSDK Install Folder>\Samples\WinUI\SideShow\Pictures
        to a working folder not under \Program Files.
     2. Open the Windows SDK CMD shell 
        (Start -> All Programs -> Windows SDK v7.0 -> SDK CMD Shell).
     3. Navigate to the directory where you copied the sample source files.
     4. Type msbuild Pictures.sln /p:platform=[win32|X64].

To build the sample using Visual Studio 2008 (preferred method):
     1. Copy source files from <PSDK Install Folder>\Samples\WinUI\SideShow\Pictures
        to a working folder not under \Program Files.
     2. Open Windows Explorer and navigate to the directory 
        where you copied the sample source files.
     3. Double-click the icon for the Pictures.sln soluion file 
        to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. 
        The sample will be built in the default \Debug or \Release directory.


Steps to run sample
-------------------
1. Register: Register the application with the Windows SideShow platform: Run the .reg registry file and select to enter the data into the registry
2. Copy the executable (WindowsSideShowPictures.exe) into a "WindowsSideShowPictures" folder on your Windows Desktop
3. Simulator: See instructions to setup and start the simulator
4. Open the Windows SideShow control panel: Control Panel -> Hardware and Sound -> Windows SideShow
5. Enable the check box for this sample application; the sample application should be run automatically by the platform
6. Copy some jpg images to your Picture directory
7. View the output on the simulator

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
A. Make sure you register the application with the appropriate .reg file (in the sample folder). Then start the simulator and enable it through the Windows SideShow control panel. See the steps to run a sample.  Examine your registry and make sure that registry entries for the sample point to correct location for the executable.

Q. Why is there no data sent to the device?
A. Ensure that the application is enabled and appears on the simulator. Ensure that you run the simulator and then run the sample application.
