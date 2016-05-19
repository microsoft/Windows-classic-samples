' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Service info (VBScript)
'
' This script displays all currently installed services.
'
'***************************************************************************

for each Service in _ 
    GetObject("winmgmts:").InstancesOf ("win32_service")
	WScript.Echo ""
	WScript.Echo Service.Name

	description = Service.Description 
	if IsNull(description) then description = "<No description>"

	pathName = Service.PathName
	if IsNull(pathName) then pathName = "<No path>"

	startName = Service.StartName
	if IsNull(startName) then startName = "<None>"

	WScript.Echo "  Description:  ", description
	WScript.Echo "  Executable:   ", pathName
	WScript.Echo "  Status:       ", Service.Status
	WScript.Echo "  State:        ", Service.State
	WScript.Echo "  Start Mode:   ", Service.StartMode
	Wscript.Echo "  Start Name:   ", startName
next

