Function Discovery Client
================================================================
First, this sample demonstrates how to use Function Discovery to create a PnP query and obtain a list of all PnP devices on the system.  It also shows how to read the metadata provided by the device.  Second, the sample demonstrates how to use Function Discovery to receive notifications from PnP.  Using Function Discovery, the application registers its query and receives a notification when a device is added or removed.

Sample Language Implementation
================================================================
This sample is available in C++.

Files
================================================================
FunDiscovery.cpp - Contains the implementation of a Function Discovery wrapper class.
FunDiscovery.h   - Contains the definition of the wrapper class.
Main.cpp         - Contains the _wmain function.

Prerequisites
================================================================
Basic understanding of C++, Win32, and Com+ programming.

To build the sample using the command prompt:
================================================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild [Solution Filename].

To build the sample using Visual Studio 2008 (preferred method):
================================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Minimum OS Version
================================================================
Windows Vista

Note
================================================================
If you add a multifunction device, such as a web camera, the sample will report an add for the camera and an add for the microphone.  This sample could easily be modified to query a Network Provider such as the SSDP or WSD provider.  

About Function Discovery
================================================================
Function Discovery provides a uniform programmatic interface for enumerating system resources, such as hardware devices, whether they are local or connected through a network. It enables applications to discover and manage lists of devices or objects sorted by functionality or class.  Function Discovery supports an extensible discovery provider model. The providers allow FD to offer an abstraction layer over existing standards such as Plug and Play (PnP), SSDP, WS-Discovery, and the registry.  For a complete listing of in-box FD providers, look at FunctionDiscoveryCategories.h.