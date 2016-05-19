' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Get application boost value (VBScript)
'
' This script demonstrates how to retrieve the info about the application boost on the local 
' machine from instances of Win32_OperatingSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_OperatingSystem")

for each System in SystemSet
	WScript.Echo "Application boost: " & System.ForegroundApplicationBoost
next
