' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Starts a service (VBScript)
'
' This script demonstrates how to start a specific service from instances of
' Win32_Service.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support services
'
'
'***************************************************************************
Set ServiceSet = GetObject("winmgmts:").ExecQuery("select * from Win32_Service where Name='ClipSrv'")

for each Service in ServiceSet
	RetVal = Service.StartService()
	if RetVal = 0 then WScript.Echo "Service started"
	if RetVal = 10 then WScript.Echo "Service already running"
next
