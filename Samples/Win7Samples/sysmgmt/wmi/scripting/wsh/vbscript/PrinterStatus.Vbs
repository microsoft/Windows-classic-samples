' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Printer status (VBScript)
'
' This script demonstrates how to retrieve printer stats from instances of
' Win32_Printer.
'
'***************************************************************************
Set PrinterSet = GetObject("winmgmts:").InstancesOf ("Win32_Printer")
If (PrinterSet.Count = 0 ) Then WScript.Echo "No Printers Installed!"
for each Printer in PrinterSet
	if Printer.PrinterStatus = 3 then WScript.Echo Printer.Name & Chr(13) & "Status:  Idle"
	if Printer.PrinterStatus = 4 then WScript.Echo Printer.Name & Chr(13) & "Status:  Printing"
	
next
