Sample Name
===============================
Windows Connect Now - WCN

Demonstrates:
===============================
WCNAPI

Introduction
===============================
Windows Connect Now (WCN) allows you to configure the wireless security on a WCN eanbled Router, discover nearby devices, and join them to the wireless network, all while maintaining a compromise between security and usability.  WCN is designed for home users, and is not suitable for enterprise scenarios.

This sample shows the basic usage of the WCNAPI’s to configure a WCN enabled device. This includes devices such as a wireless router or printer.  Also, this demonstrates getting a wireless profile from a configured WCN enabled router via the devices' push button.
 

Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

Files
===============================
WcnConfigure.cpp – Main entry point for the sample.  This is where the command line parsing and the WCNAPI calls originate

WcnConnectNotify.cpp – Helper file that implements the IWCNConnectNotify Interface.  WCN uses this interface to notify the application of WCN connection events.

WcnFdHelper.cpp – Helper file that implements and wraps the Function Discovery Interface that is used by WCN to Find a WCN Device.

Prerequisites
===============================
	- This sample app needs to run as an elevated user.
 	- ATL is required to compile this Sample
	- Wireless Adapter present on the system.
	- Only runs on Windows 7 
	- This sample is command line only

To build the sample using the command prompt:
===============================
	1. Open the Command Prompt window and navigate to the  directory.
	2. Type msbuild WindowConnectNow.sln

To build the sample using Visual Studio 2005 (preferred method):
===============================
	1. Open Windows Explorer and navigate to the  directory.
	2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
	3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


Running the Sample
===============================

To run the sample:

	1. From an elevated command prompt navigate to the directory that contains the new executable.
	2. Type the following at the command line, depending on what you want to do.

	WCNConfigure.exe – without any options this will display the following usage.
USAGE:
 WCNConfigure.exe
  Scenario=[DeviceConfigPin | DeviceConfigPushButton | RouterConfig |
            PCConfigPushButton | PCConfigPin ]
  [UUID=<uuid of device> | SEARCHSSID=<ssid of device to find>]
  [PIN=<pin of device>]
  [PROFILESSID=<ssid to use in profile>]
  [PROFILEPASSPHRASE=<passphrase to use in profile>]

Parameters:
 Scenario - choose the operation you wish to perform
     DeviceConfigPushButton - Configure a WCN enabled device, such as a picture
                              frame using the button on the device
     DeviceConfigPin - Configure a WCN enabled device, such as a picture frame
                       using the device supplied pin
     RouterConfig - Configure a WCN enabled Wireless Router
     PCConfigPushButton - Get the wireless profile from a WCN enabled router
                          using the Push Button on the device.
     PCConfigPin - Get the wireless profile from a WCN enabled rotuer using the
                   supplied pin.

 UUID - Enter a device UUID in the following format xxxx-xxxx-xxxx-xxxxxxxxxxxx
        UUID is necessary for the DeviceConfigPushButton and DeviceConfigPin
        scenarios. Use either UUID or SEARCHSSID for the RouterConfig, PCConfigPin
        and PCConfigPushButton scenarios.

 SEARCHSSID - Enter in the SSID for the Router you are looking to configure.
              SEARCHSSID is only valid in the RouterConfig, PCConfigPushButton and
              PCConfigPin scenarios. Use either UUID or SEARCHSSID for the these
              scenarios. NOTE: Using SSID will return the first device
              found with that ssid.  If there is more than one device with the
              same ssid use the UUID instead

 PIN  - Enter the pin of the device
        PIN is only valid when using the RouterConfig and DeviceConfigPIN
        Scenarios.

 PROFILESSID - When present this SSID will be used in the WLAN profile that is
               pushed to the router/device otherwise a default SSID of WCNSSID
               will be used

 PROFILEPASSPHRASE - when present this passphrase will be used in the wlan
                     profile that is pushed to the router/device. Otherwise, a
                     random default passphrase will be used



Example: 
===============================  
The following will find the Router based on its UUID and configure it using the supplied pin. It will then save the wireless profile it used to configure the router to the system.

C:\>WCNConfigure.exe Scenario=RouterConfig UUID=12345678-1234-1234-1234-123456789012 PIN=12345670

INFO: Stating the Function Discovery Search...
INFO: The following Device was found by Function Discovery.
INFO: Device Name: [Device Name]
INFO: Manufacturer Name: [Manufacturer name]
INFO: Model Name: [Model Name]
INFO: Model Number: [Model number]
INFO: Serial Number: [serial number]
INFO: Successfully saved the profile to the wlan store
INFO: Successfully retrieved profile [WCNSSID] from the wlan store.
INFO: IWCNDevice::SetNetworkProfile() succeeded with result [0x0]
INFO: IWCNDevice::SetPassword succeeded with result [0x0]
INFO: IWCNDevice::Connect succeeded with result [0x0]
INFO: IWCNConnectNotify::ConnectSucceeded was invoked
INFO: Profile SSID Used: [WCNSSID]
INFO: Profile Passphrase Used: [1234-abcd-1d2a]
INFO: Configuration of the Wireless Router Succeeded



