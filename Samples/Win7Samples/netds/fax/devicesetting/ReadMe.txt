========================================================================
   SAMPLE : Device Setting
========================================================================

Description:
-----------------

This sample demonstrates the use Device APIs available with Microsoft Fax. It includes listing the available devices on a fax server and 
setting the TSID or CSID of a device.

A device here means a modem that is fax capable.

PreCondition:
-------------------

Listing of device  will work if the user has Query Config access right and is run in elevated mode.
Setting of TSID and CSID will work if the user has Manage Config access right and is run in elevated mode.

This sample is supported for Windows Vista.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To get a list of devices on the fax server : DeviceSettingCPP.exe /s <FaxServerName> /l list 

To set the TSID of a device: DeviceSettingCPP.exe /s <FaxServerName> /l set /i DeviceId /t TSID

To set the CSID of a device: DeviceSettingCPP.exe /s <FaxServerName> /l set /i DeviceId /c CSID

DeviceId is the id printed by the /l list option.

If /s paramater is not given then the default Fax Server is the local server. 