RegDevice Sample UPnP Application in C++


Overview:
=========

RegDevice implements a dimmer device functionality as a COM server object
and demonstrates how to register/unregister this device with the Microsoft 
UPnP framework's Device Host. The device specific functionality is implemented
in the ..\DCO_DimmerService directory.


Files:
======

DimmerDevice-Desc.xml	- Device description document for the Dimmer device type
DimmingService_SCPD.xml	- Service description document for the Dimmer service type
RegDevice.cpp		- Implements the dimmer device registration/unregistration
                          with the UPnP framework's Device Host.			


Building the Sample:
====================

Build the ..\DCO_DimmerService directory first using the steps listed in
the readme.txt of the DCO_DimmerService directory.


To build the sample using the command prompt:

     1. Open the Command Prompt window and navigate to the RegisterDevice directory.
     2. Type 'msbuild RegDevice.sln'

To build the sample using Visual Studio 2008 (preferred method):

     1. Open Windows Explorer and navigate to the RegisterDevice directory.
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

