' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - NT Event Logger (VBScript)
'
' This script demonstrates how to display NT events using a notification
' query.
'
'***************************************************************************
on error resume next
set locator = CreateObject("WbemScripting.SWbemLocator")

'Access to the NT event log requires the security privilege
locator.Security_.Privileges.AddAsString "SeSecurityPrivilege"

set events = locator.ConnectServer().ExecNotificationQuery _ 
	("select * from __instancecreationevent where targetinstance isa 'Win32_NTLogEvent'") 					

if err <> 0 then
	WScript.Echo Err.Description, Err.Number, Err.Source
end if 

' Note this next call will wait indefinitely - a timeout can be specified 

WScript.Echo "Waiting for NT Events..."
WScript.Echo ""

do 
	set NTEvent = events.nextevent 
	if err <> 0 then
		WScript.Echo Err.Number, Err.Description, Err.Source
		Exit Do
	elseif  NTEvent.TargetInstance.Message <> Empty then		
		WScript.Echo NTEvent.TargetInstance.Message
	else
		Wscript.Echo "Event received, but it did not contain a message."
	end if
loop

WScript.Echo "finished"

