OSD: Windows Audio Volume On Screen Display Sample (Win32, C++)

Copyright (c) Microsoft Corporation. All Rights Reserved



What does the sample do?
=========================

    This sample is a Win32-based application that demonstrates the use of the Vista APIs for monitoring the default audio output device and its current volume setting. The sample is written in C++.

    OSD does *not* run on earlier versions of Windows, including Windows XP, Windows 2000, Windows Me, and Windows 98.


How to build the sample
=========================

    Install the Platform SDK. 
    
    Open the CMD shell for the Windows SDK and change to the OSD sample directory. Run the command "start OSD.sln" in the OSD directory to open the OSD project in the Visual Studio window. From within the window, select the Debug or Release solution configuration, select the Build menu from the menu bar, and select the Build option. Note that if you do not open Visual Studio from the CMD shell for the SDK, Visual Studio will not have access to the SDK build environment. In that case, the sample will not build unless you explicitly set environment variable MSSdk, which is used in the project file, OSD.vcproj.


How to run the sample
=========================

    Run the OSD executable file, OSD.exe, in Windows Vista.  Note that you will not see a system tray icon or a window for the application, but you can see the process running using TaskMgr.exe.

    Run sndvol.exe to change the volume or mute, or change the volume using keyboard controls or a HID control.  And OSD should be displayed.
    
    To exit the application, run TaskMgr.exe, hilight the OSD.exe process and click "End Process".


Targeted platforms
=========================

    OSD runs on 32-bit and 64-bit Windows Vista platforms.
    

APIs used in the sample
=========================

    - MMDevice API (multimedia device enumeration and selection)
    - Audio Endpoint Volume API

Known limitations
=========================

        
File manifest
=========================

stdafx.h            -- Precompiled header generation
stdafx.cpp          --  "
OSD.h               -- Common header
OSD.cpp             -- WinMain and windows proc
endpointMonitor.h   -- Declaration of endpoint and volume monitoring class
endpointMonitor.cpp -- Implementation of endpoint and volume monitoring class
OSD.rc              -- Version info
OSD.sln             -- Visual Studio 2005 solution file
OSD.vcproj          -- Visual Studio 2005 project file
