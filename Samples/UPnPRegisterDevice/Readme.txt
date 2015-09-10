RegDevice Sample UPnP Application in C++


Overview:
=========

RegDevice implements a dimmer device functionality as a COM server object
and demonstrates how to register/unregister this device with the Microsoft 
UPnP framework's Device Host. The device specific functionality is implemented
in the ..\DCO_DimmerService directory.
This sample also demonstrates usage of a VBScript based presentation webpage
for the Dimmer Device. The DimmerPresentation.htm file contains the presentation
webpage for the dimmer device. This webpage can be used to control the device
by invoking action on the device using a web browser. 


Files:
======

DimmerDevice-Desc.xml	- Device description document for the Dimmer device type
DimmingService_SCPD.xml	- Service description document for the Dimmer service type
RegDevice.cpp		- Implements the dimmer device registration/unregistration
                          with the UPnP framework's Device Host.	
DimmerPresentation.htm  - Presentation webpage


Building the Sample:
====================

Build the ..\DCO_DimmerService directory first using the steps listed in
the readme.txt of the DCO_DimmerService directory.


To build the sample using the command prompt:

     1. Open the Command Prompt window and navigate to the RegisterDevice directory.
     2. Type 'msbuild RegDevice.sln'

To build the sample using Visual Studio 2011 (preferred method):

     1. Open File Explorer and navigate to the RegisterDevice\cpp directory.
     2. Double-click the icon for the RegDevice.sln (solution) file to open the file 
        in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the 
        default \Debug or \Release directory.


Running the Sample:
====================

     1. Open a Command Prompt and navigate to the Release or Debug directory under
        DCO_DimmerService.
     2. Run 'regsvr32 UPNPSampleDimmerDevice.dll' to register the device specific
        COM object.
     3. Navigate to the Release or Debug directory under RegisterDevice
     4. Copy the DimmerDevice-Desc.xml and DimmingService_SCPD.xml files from 
        the RegisterDevice directory to the current directory.
     5. Run RegDevice.exe
     6. Once the device is hosted, go to "My Computer" of a Windows 8 based PC.
     7. Click "Network" in the left-side menu.
     8. Turn-On Network Discovery if prompted.
     9. The "UPNP SDK Dimmer Device Hosted by Windows" should appear under "
        Other Devices"
     10.Double Click the device in Network Explorer.
     11.This should open a presentation webpage in a web browser.
     12.Accept the security warning in order to be able to control the device.
     13.Now, interface on the presentation webpage can be used to control the 
        device.


