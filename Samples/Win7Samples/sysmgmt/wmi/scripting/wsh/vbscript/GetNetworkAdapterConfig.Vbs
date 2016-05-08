' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List network adapter configuration (VBScript)
'
' This script demonstrates how to retrieve configuration info from an instance of
' Win32_NetworkAdapterConfiguration.  In this case we specify the adapter with an Index
' of 0, which under Windows 2000 should be your ethernet card.  If you would like to enumerate a 
' different instance, or if the given Index is not for your ethernet adapter, replace the Index in the 
' GetObject line.  The correct Index should be selected from Win32_NetworkAdapter instances 
' for other interfaces.
'
'***************************************************************************
Set Adapter = GetObject("winmgmts:Win32_NetworkAdapterConfiguration=""0""")

if isnull(Adapter.MACAddress) then
	MAC = "No MAC Address"
else
	MAC = Adapter.MACAddress
end if
if isnull(Adapter.IPAddress) then
	IP = "No IP Address"
else
	IP = Adapter.IPAddress(0)
end if

Description = Adapter.Description

if Adapter.DHCPEnabled = true then 
	WScript.Echo " Adapter:", Description, Chr(13),_
		"IP:", IP, Chr(13),_
		"MAC", MAC, Chr(13), _
		"DHCP Enabled"
else 
	WScript.Echo " Adapter:", Description, Chr(13),_
		"IP:", IP, Chr(13),_
		"MAC", MAC, Chr(13), _
		"DHCP Disabled"
end if
