' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - System shutdown (VBScript)
'
' Invokes the Shutdown method of the Win32_OperatingSystem class
' NOTE:  You must have the Shutdown privilege to successfully invoke the Shutdown method
'
'***************************************************************************
Set OpSysSet = GetObject("winmgmts:{(Shutdown)}//./root/cimv2").ExecQuery("select * from Win32_OperatingSystem where Primary=true")

for each OpSys in OpSysSet
	OpSys.Shutdown()
next