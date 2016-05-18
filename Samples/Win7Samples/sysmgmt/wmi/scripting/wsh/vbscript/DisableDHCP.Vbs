' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Disable DHCP on adapter (use static IP addr) (VBScript)
'
' This script demonstrates how to disable DHCP use on an instance of
' Win32_NetworkAdapterConfiguration.  In this case we specify the adapter with an Index
' of 0.  The correct index should be selected from Win32_NetworkAdapter instances for other
' interfaces.
'
'
' NOTE:  This script only applies to NT-based systems
' NOTE:  Change the ipaddr and subnet variables below to the values you wish to apply
'        to the adapter.
'
'
'***************************************************************************
Set Adapter = GetObject("winmgmts:Win32_NetworkAdapterConfiguration=1")

ipaddr = Array("1.1.1.1")
subnet = Array("255.255.255.0")


RetVal = Adapter.EnableStatic(ipaddr,subnet)

if RetVal = 0 then 
	WScript.Echo "DHCP disabled, using static IP address"
else 
	WScript.Echo "DHCP disable failed"
end if

