' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List network adapters (VBScript)
'
' This script demonstrates how to retrieve a list of adapters from instances of
' Win32_NetworkAdapterConfiguration.
'
'***************************************************************************
Set AdapterSet = GetObject("winmgmts:").InstancesOf("Win32_NetworkAdapterConfiguration")

for each Adapter in AdapterSet
	if Adapter.IPEnabled = "True" then
		WScript.Echo "Adapter:", Adapter.Description
		WScript.Echo "IP:", Adapter.IPAddress(0)
		WScript.Echo "MAC", Adapter.MACAddress
	end if
next
