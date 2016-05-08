' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Date and time (VBScript)
'
' This script demonstrates how to retrieve the date and time of the local machine 
' from instances of Win32_OperatingSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_OperatingSystem")

for each System in SystemSet
	WScript.Echo System.LocalDateTime
next
