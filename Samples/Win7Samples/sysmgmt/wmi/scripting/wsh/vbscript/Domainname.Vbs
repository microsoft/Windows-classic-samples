' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Domain name (VBScript)
'
' This script demonstrates how to retrieve the domain name of the local machine 
' from instances of Win32_ComputerSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_ComputerSystem")

for each System in SystemSet
	WScript.Echo System.Domain
next
