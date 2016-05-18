' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Machine name (VBScript)
'
' This script demonstrates how to retrieve the local machine name from instances of
' Win32_ComputerSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_ComputerSystem")

for each System in SystemSet
	WScript.Echo System.Name
next
