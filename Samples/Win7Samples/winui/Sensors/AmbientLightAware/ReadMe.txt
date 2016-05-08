=========================
Ambient Light Aware
Windows 7 Sensor Platform
=========================
This MFC sample shows how to use the Sensor Platform by reading data from Ambient Light Sensors on the computer and changing a label's font size to be optimized for the reported light conditions.  Such as smaller font while inside and larger font while outside in a sunny environment.  The sample is not meant to be perfect in optimizing for light conditions, but done to show the usage of the Sensor Platform.  This sample requires Visual Studio to be built.

===============================
Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

=====
Files
=====
AmbientLightAware.cpp                     The main CWinApp file
AmbientLightAware.h                       Header file
AmbientLightAware.rc                      Resource file for MFC
AmbientLightAware.sln                     Solution for the project
AmbientLightAware.vcproj                  VS Project file
AmbientLightAwareDlg.cpp                  Dialog class
AmbientLightAwareDlg.h                    Header file
AmbientLightAwareSensorManagerEvents.cpp  Custom class to handle ISensorManager events
AmbientLightAwareSensorManagerEvents.h    Header file
AmbientLightAwareSensorEvents.cpp         Custom class to handle ISensor events
AmbientLightAwareSensorEvents.h           Header file
ReadMe.txt                                This ReadMe
resource.h                                Resource file
stdafx.cpp                                Precompiled file
stdafx.h                                  Precompiled header

============= 
Prerequisites
=============
Windows 7
Visual Studio 2008

========
Building
========
This sample cannot be built in the SDK environment.  Visual Studio must be used.
To build the sample using Visual Studio 2008:
     1. Run the Windows SDK Configuration Tool provided with the SDK to add SDK include directories to Visual Studio
     2. Open Windows Explorer and navigate to the directory.
     3. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     4. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type AmbientLightAware.exe at the command line, or double-click the icon for AmbientLightAware.exe to launch it from Windows Explorer.

To run from Visual Studio:
     1. Press F5 or Click menu
           Debug->Start Debugging