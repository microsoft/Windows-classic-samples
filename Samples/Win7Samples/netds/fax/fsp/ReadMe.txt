========================================================================
   SAMPLE : Fax Sample FSP
========================================================================

Description:
-----------------
	This sample demonstrates the use of Fax Service Provider APIs. SampleFSP.dll is a sample Windows Fax virtual service provider.


How to install
-------------------
	"regsvr32 SampleFSP.dll", be sure to run this command from the directory of the SampleFSP.dll

Configuration
-------------------

	SampeFSP Configuration: 
	-------------------------------------
	The SampleFSP configuration settings are found under "SampleFSP: Sample Windows Fax Service Provider" in the registry under the Fax Device Providers key.

	The available configuration settings are:
	- LoggingEnabled:   0 - logging disabled, 1 - logging enabled
	This is initially set to 0
	- LoggingDirectory: A valid directory
	This is initially set to the directory from which "regsvr32 SampleFSP.dll" was run

	Device Settings
	----------------------
	The SampleFSP device settings are found under "Devices\<Device Id>" in the registry under the SampleFSP key.

	The available configuration settings are:
	- Directory: A valid directory
	This is initially set to the directory from which "regsvr32 SampleFSP.dll" was run

Usage
----------
	To send a fax
	-------------------
	Set the fax number to "Dial exactly as typed" and don't use a dialing rule.
	As the dialed number, select the destination device number, for example: if you wish to dial to device "SampleFSP Device 1", just dial: 1.
	Be sure that you have a SampleFSP device configured for sending and that the destination device is configured for auto receive (it is recommended to disable all other non SampleFSP device, so they will not 		send the faxes).
	The fax will be copied to the associated device directory (as stated in the registry under that device key). The directory should have write access.

	To receive a fax
	-----------------------
	Change the file attributes of a file in the directory associated with the device.  The first tiff file found in that directory will be copied to the received fax.

Limitations
----------------
	- There is minimal directory validation.
	- There is no tiff validation.

