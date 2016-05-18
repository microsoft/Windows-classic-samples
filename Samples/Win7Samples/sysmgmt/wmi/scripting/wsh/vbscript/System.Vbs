' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Win32_ComputerSystem dump (VBScript)
'
' This script demonstrates how to dump properties from instances of
' Win32_ComputerSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_ComputerSystem")

for each System in SystemSet
	WScript.Echo System.Caption
	WScript.Echo System.PrimaryOwnerName
	WScript.Echo System.Domain
	WScript.Echo System.SystemType
next
