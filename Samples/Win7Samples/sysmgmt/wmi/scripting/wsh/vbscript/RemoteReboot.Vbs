' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - REMOTE system reboot (VBScript)
'
' Invokes the Reboot method of the Win32_OperatingSystem class
' Fill in REMOTE_SYSTEM_NAME with the name of the remote system to reboot
' NOTE:  You must have the RemoteShutdown privilege to successfully invoke the Reboot method
'
'***************************************************************************
Set OpSysSet = GetObject("winmgmts:{(RemoteShutdown)}//REMOTE_SYSTEM_NAME/root/cimv2").ExecQuery("select * from Win32_OperatingSystem where Primary=true")

for each OpSys in OpSysSet
	OpSys.Reboot()
next