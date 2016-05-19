' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List event log files (VBScript)
'
' This script demonstrates how to retrieve the info about the event log files 
' on the local machine from instances of Win32_NTEventLogFile.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support event logs
'
'***************************************************************************
Set LogFileSet = GetObject("winmgmts:").InstancesOf ("Win32_NTEventLogFile")

for each Logfile in LogFileSet
	WScript.Echo " Log Name: " & Logfile.LogfileName & Chr(13), _
		"Number of Records: " & Logfile.NumberOfRecords & Chr(13), _
		"Max Size: " & Logfile.MaxFileSize & " bytes" & Chr(13), _
		"File name: " & Logfile.Name
next
