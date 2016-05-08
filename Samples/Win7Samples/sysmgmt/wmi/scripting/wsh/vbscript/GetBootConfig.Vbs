' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Current boot configuration (VBScript)
'
' This script demonstrates how to retrieve the current boot configuration setting of the local machine 
' from instances of Win32_OperatingSystem.
'
'***************************************************************************
Set BootSet = GetObject("winmgmts:").InstancesOf ("Win32_OperatingSystem")

for each Boot in BootSet
	WScript.Echo Boot.Name
next
