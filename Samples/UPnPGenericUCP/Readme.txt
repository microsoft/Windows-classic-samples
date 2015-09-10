
GenericUCP Sample UPnP Application in C++


Overview:
=========

GenericUCP is a C++ sample UPnP application that discovers and controls 
UPnP devices found on the network.This sample was developed using Visual C++ 
and uses MFC.

The sample application uses the UPnP Control Point API provided by upnp.dll. The 
application can discover devices on the network through one of the three types
of searches available: FindByType, FindByUDN and AsyncFind.

The devices found are instantiated in the device list. When a device is 
selected, service objects for the device are enumerated and listed in the 
service list. If the Delay Subscription checkbox is checked when the desired
device is selected, the Subscribe button will become available and SCPD 
download and event subscription will be delayed when the services are 
enumerated. If the Delay Subscription checkbox is not checked, the SCPD will be 
downloaded and the subscriptions will be done while enumerating services.

One of the services can be selected and controlled by invoking actions against
it. The events relevant to the service are displayed in the events field. If 
the Delay Subscription checkbox was checked when the device was selected, 
Subscribe will need to be used to start event subscriptions. If Asynchronous 
Control is selected, the asynchronous control methods will be used. If the 
Asynchronous Control box is not selected, the normal synchronous methods will 
be used.


Files:
======

CAsyncResult.h  - Implements CUPnPAsyncResult, which provides an asynchronous callback to 
                  the client and is an implementation of the IUPnPAsyncResult object.
deviceprop.cpp	- Implements CDeviceProp which is a dialog box for printing the 
				  IUPnPDevice objects's properties
deviceprop.h 	- Definitions for CDeviceProp class 
devtype.txt	    - Device types used by GenericUCP.exe in the dropdown menu
genericucp.cpp 	- Provides the CWinApp implementation for this sample application
genericucp.h	- Definitions for CGenericUCPApp which is derived from CWinApp
genericucp.ico	- Icon file used by this sample application
genericucp.rc	- Resource for this application
GenericUCPDlg.cpp - Demonstrates the use of various UPnP control point APIs
genericucpdlg.h	- Class definitions for the above file
resource.h	    - Resource include file
SCPDDisplay.cpp - Implements CSCPDDisplay which is a dialog box for printing 
                  the Service Description Document
SCPDDisplay.h   - Definitions for CSCPDDisplay class
stdafx.cpp	    - Source file that includes the standard includes 
stdafx.h	    - Standard Include file
udn.txt 	    - contains UDNs used by GenericUCP.exe in the dropdown menu
util.cpp	    - Utility routines used by this sample
util.h		    - Declarations for the utility routines




Building the Sample:
====================

To build the sample using the command prompt:

	 1. Open the Command Prompt window and navigate to the GenericUCP directory.
	 2. Type 'msbuild genericucp.sln'

To build the sample using Visual Studio 2011 (preferred method):

	 1. Open File Explorer and navigate to the GenericUCP\cpp directory.
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

This sample only functions for device supported actions. Multiple argument 
Invoke Action arguments are separated by a space.

The sample uses most of the UPnP control point APIs. The comments in the code give more 
detail on usage of these APIs. This sample does not show the usage of some of the APIs 
like IUPnPDescriptionDocument interface APIs, IUPnPDeviceDocumentAccess interface APIs, etc. 


