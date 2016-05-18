' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List event log events of a particular type (VBScript)
'
' This script demonstrates how to retrieve the events of a particular type (Event code)
' from the Win32_NTLogEvent class
'
' NOTE:  This script only applies to NT-based systems since Win9x does support event logs
'
'
'***************************************************************************
Set EventSet = GetObject("winmgmts:").ExecQuery("select * from Win32_NTLogEvent where EventCode=4097")

if (EventSet.Count = 0) then WScript.Echo "No Events"

for each LogEvent in EventSet
	WScript.Echo "Event Number: " & LogEvent.RecordNumber
	WScript.Echo "Log File: " & LogEvent.LogFile
	WScript.Echo "Type: " & LogEvent.Type 
	WScript.Echo "Message: " & LogEvent.Message
	WScript.Echo "Time: " & LogEvent.TimeGenerated
	WScript.Echo ""
next
