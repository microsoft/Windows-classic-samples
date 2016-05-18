========================================================================
   SAMPLE : Fax Outbound Routing
========================================================================

Description:
-----------------

This sample demonstrates the use of Fax Outbound Routing  APIs. It includes creating groups and rules, deleting them and listing them. 

PreCondition:
-------------------

Setting of Fax Outbound Routing will work 
1. If the user has "Query Server Config" and "Manage Server Config" ACE 
2. Running in elevated mode.

This is supported for Windows Vista Server SKU.

Note: For C# and VB.Net Samples to run, you need to copy the managed FaxComexLib.dll to the same folder as the exe.

Usage:
---------

To get the list of Routing Groups on the server: OutboundRoutingCPP.exe /s <FaxServerName> /o listgroups

To get the list of Routing Rules on the server: OutboundRoutingCPP.exe /s <FaxServerName> /o listrules
 
To delete a Routing Group from the server: OutboundRoutingCPP.exe /s <FaxServerName> /o removeGroup /i <n>
where n is a value from 1 to the number of groups on the fax server

To delete a Routing Rule from the server: OutboundRoutingCPP.exe /s <FaxServerName> /o removeRule  /i <n>
where n is a value from 1 to the number of rules on the fax server

To add a new Routing Group to the Fax Server: OutboundRoutingCPP.exe /s <FaxServerName> /o addGroup /n GroupName /d <FaxDeviceIds>
where FaxDeviceIds is a ";" separated string of device id of the Fax Modems. E.g. "65538;12344"

To add a new Routing Rule to the Fax Server for a Device: OutboundRoutingCPP.exe /s <FaxServerName> /o addRule /b 1 /d <FaxDeviceId> /c 1 /a 22 
where FaxDeviceId is the device id of the Fax Modem. E.g. "65545"
/c: Country code for the rule
/a: Area Code for the rule.
/b: can be 0 or 1 when 0 the rule is applied on the Group Name and when 1 the rule is applied on a particular device.

To add a new Routing Rule to the Fax Server for a Routing Group: OutboundRoutingCPP.exe /s <FaxServerName> /o addRule /b 0 /n GroupName /c 1 /a 44
/c: Country code for the rule
/a: Area Code for the rule.
/b: can be 0 or 1 when 0 the rule is applied on the Group Name and when 1 the rule is applied on a particular device.

If /s paramater is not given then the default Fax Server is the local server. 
DeviceId of a Fax Modem can be found by list the routing groups on the server or the sample DeviceSetting for finding the Fax Device Id.