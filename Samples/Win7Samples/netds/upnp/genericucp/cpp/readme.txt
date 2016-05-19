
GenericUCP Sample UPnP Application in C++


Overview:
=========

GenericUCP is a C++ sample UPnP application that discovers and controls 
UPnP devices found on the network.This sample was developed using Visual C++ 
and uses MFC.

The sample application uses the UPnP Control Point API provided by upnp.dll. The 
application can discover devices on the network through one of the three types
of searches available: FindByType, FindByUDN and AsyncFind. 

The devices found are instantiated in the device list. One of the devices can be 
selected and the service objects for the selected device are listed in the service
list. One of the services can be selected and controlled by invoking actions against
it. The events relevant to the service are displayed in the events field. 


Files:
======

deviceprop.cpp	- Implements CDeviceProp which is a dialog box for printing the 
                  IUPnPDevice objects's properties
deviceprop.h 	- Definitions for CDeviceProp class 
devtype.txt	- Device types used by GenericUCP.exe in the dropdown menu
genericucp.cpp 	- Provides the CWinApp implementation for this sample application
genericucp.h	- Definitions for CGenericUCPApp which is derived from CWinApp
genericucp.ico	- Icon file used by this sample application
genericucp.rc	- Resource for this application
GenericUCPDlg.cpp - Demonstrates the use of various UPnP control point APIs
genericucpdlg.h	- Class definitions for the above file
resource.h	- Resource include file
stdafx.cpp	- Source file that includes the standard includes 
stdafx.h	- Standard Include file
udn.txt 	- contains UDNs used by GenericUCP.exe in the dropdown menu
util.cpp	- Utility routines used by this sample
util.h		- Declarations for the utility routines




Building the Sample:
====================

To build the sample using the command prompt:

     1. Open the Command Prompt window and navigate to the GenericUCP directory.
     2. Type 'msbuild genericucp.sln'

To build the sample using Visual Studio 2008 (preferred method):

     1. Open Windows Explorer and navigate to the GenericUCP directory.
     2. Double-click the icon for the genericucp.sln (solution) file to open the file 
        in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the 
        default \Debug or \Release directory.



Running the Sample:
===================

     1. Open a Command Prompt and navigate to the Release or Debug directory under 
        GenericUcp.
     2. Copy the devType.txt and Udn.txt files from the CPP directory to the current
        directory. 
     3. Run GenericUcp.exe. The device types and the UDNs that appear in the drop
	down menu are from devType.txt and Udn.txt respectively.



Comments:
=========

The sample uses most of the UPnP control point APIs. The comments in the code give more 
detail on usage of these APIs. This sample does not show the usage of some of the APIs 
like IUPnPDescriptionDocument interface APIs, IUPnPDeviceDocumentAccess interface APIs, etc. 


