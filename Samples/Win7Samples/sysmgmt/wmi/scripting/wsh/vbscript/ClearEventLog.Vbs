' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Clear event log (VBScript)
'
' This script demonstrates how to clear the entries from the System event log file
' on the local machine from instances of Win32_NTEventLogFile.
'
' NOTE:  A backup file can be specified to the ClearEventlog() method if a backup of
' the data is desired before clearing the log.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support event logs
'
'
'***************************************************************************
Set LogFileSet = GetObject("winmgmts:{(Backup,Security)}").ExecQuery("select * from Win32_NTEventLogFile where LogfileName='System'")

for each Logfile in LogFileSet
	RetVal = LogFile.ClearEventlog()
	if RetVal = 0 then WScript.Echo "Log Cleared"
next
