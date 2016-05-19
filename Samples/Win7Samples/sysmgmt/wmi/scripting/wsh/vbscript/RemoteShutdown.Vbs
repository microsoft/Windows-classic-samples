' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - REMOTE system shutdown (VBScript)
'
' Invokes the Shutdown method of the Win32_OperatingSystem class
' Fill in REMOTE_SYSTEM_NAME with the name of the remote system to shutdown
' NOTE:  You must have the RemoteShutdown privilege to successfully invoke the Shutdown method
'
'***************************************************************************
Set OpSysSet = GetObject("winmgmts:{(Debug,RemoteShutdown)}//REMOTE_SYSTEM_NAME/root/cimv2").ExecQuery("select * from Win32_OperatingSystem where Primary=true")

for each OpSys in OpSysSet
	OpSys.Shutdown()
next