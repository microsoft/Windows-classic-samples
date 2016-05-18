' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - System reboot (VBScript)
'
' Invokes the Reboot method of the Win32_OperatingSystem class
' NOTE:  You must have the Shutdown privilege to successfully invoke the Shutdown method
'
'***************************************************************************
Set OpSysSet = GetObject("winmgmts:{(Shutdown)}//./root/cimv2").ExecQuery("select * from Win32_OperatingSystem where Primary=true")

for each OpSys in OpSysSet
	OpSys.Reboot()
next