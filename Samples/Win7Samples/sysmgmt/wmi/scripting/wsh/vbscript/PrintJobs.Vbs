' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List print jobs (VBScript)
'
' This script demonstrates how to retrieve printer jobs stats from instances of
' Win32_PrintJob.
'
'***************************************************************************
Set PrintJobSet = GetObject("winmgmts:").InstancesOf ("Win32_PrintJob")

If (PrintJobSet.Count = 0) Then WScript.Echo "No print jobs!"
for each PrintJob in PrintJobSet
	WScript.Echo PrintJob.Name
	WScript.Echo PrintJob.JobId
	WScript.Echo PrintJob.Status
	WScript.Echo PrintJob.TotalPages
	Wscript.Echo ""
next
