' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Information about the OS (VBScript)
'
' This script demonstrates how to retrieve the info about the OS on the local machine from instances of
' Win32_OperatingSystem.
'
'***************************************************************************
Set SystemSet = GetObject("winmgmts:").InstancesOf ("Win32_OperatingSystem")

for each System in SystemSet
	WScript.Echo System.Caption 
	WScript.Echo System.Manufacturer
	WScript.Echo System.BuildType
	WScript.Echo " Version: " + System.Version
	WScript.Echo " Locale: " + System.Locale
	WScript.Echo " Windows Directory: " + System.WindowsDirectory
	WScript.Echo " Total memory: " + System.TotalVisibleMemorySize + " bytes"
	WScript.Echo " Serial Number: " + System.SerialNumber
	Wscript.Echo ""
next
