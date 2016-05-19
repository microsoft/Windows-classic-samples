' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Enable DHCP on adapter (VBScript)
'
' This script demonstrates how to enable DHCP use on an instance of
' Win32_NetworkAdapterConfiguration.  In this case we specify the adapter with an Index
' of 0.  The correct index should be selected from Win32_NetworkAdapter instances for other
' interfaces.
'
' Supported on NT platforms only
'
'***************************************************************************
Set Adapter = GetObject("winmgmts:Win32_NetworkAdapterConfiguration=0")

RetVal = Adapter.EnableDHCP()

if RetVal = 0 then 
	WScript.Echo "DHCP Enabled"
else 
	WScript.Echo "DHCP enable failed"
end if
