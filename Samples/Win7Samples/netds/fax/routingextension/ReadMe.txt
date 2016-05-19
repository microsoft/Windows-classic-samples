========================================================================
   SAMPLE : Fax Routing Extension
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Routing Extension APIs. It has a Routing Method that when enabled for a device will make a copy of faxes received on that device. 
The location of the copied faxes is %SystemDrive%\SampleRouteFolder.


PreCondition:
-------------------

This is supported for Windows Vista Server SKU.

Copy the dll to %windir%\system32

Usage:
---------

To register the Routing Extension: Use regsvr32 FaxRouteIt.dll and then restart Fax Service
To unregister the Routing Extension: Use regsvr32 /u FaxRouteIt.dll and then restart Fax Service

Enable the routing extension for the devices and then receive faxes.
Check %systemdrive%\SampleRouteFolder for copies of the received files.

Routing Extension can be set for local fax server.