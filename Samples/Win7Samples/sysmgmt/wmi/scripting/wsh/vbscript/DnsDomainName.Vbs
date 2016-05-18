' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - DNS Domain name (VBScript)
'
' This script demonstrates how to retrieve the DNS domain name of the local machine 
' from instances of Win32_ComputerSystem.
'
'***************************************************************************
Set NICSet = GetObject("winmgmts:").ExecQuery ("select * from Win32_NetworkAdapterConfiguration where IPEnabled=true")

for each NIC in NICSet
	WScript.Echo NIC.DNSDomain
next
