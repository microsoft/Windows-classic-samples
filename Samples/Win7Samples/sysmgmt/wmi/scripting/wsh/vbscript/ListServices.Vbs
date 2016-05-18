' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List services (VBScript)
'
' This script demonstrates how to retrieve a list of running services from instances of
' Win32_Service.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support services
'
'
'***************************************************************************
Set ServiceSet = GetObject("winmgmts:").ExecQuery("select * from Win32_Service where State=""Running""")

for each Service in ServiceSet
	WScript.Echo Service.Name

	description = Service.Description

	if IsNull (description) then
		WScript.Echo " <No Description>"
	else
		WScript.Echo " Description: " & Service.Description 
	end if

	WScript.Echo " Process ID: " & Service.ProcessId
	WScript.Echo " Start Mode: " & Service.StartMode
	WScript.Echo ""
next


