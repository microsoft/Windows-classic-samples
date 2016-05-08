' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Backup event log (VBScript)
'
' This script demonstrates how to backup the entries from the Application event log file
' on the local machine from instances of Win32_NTEventLogFile.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support event logs
'
'
'***************************************************************************
Set LogFileSet = GetObject("winmgmts:{(Backup,Security)}").ExecQuery("select * from Win32_NTEventLogFile where LogfileName='Application'")

for each Logfile in LogFileSet
	RetVal = LogFile.BackupEventlog("c:\BACKUP.LOG")
	if RetVal = 0 then WScript.Echo "Log Backed Up"
next
