' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Resumes a service (VBScript)
'
' This script demonstrates how to resume a paused service from instances of
' Win32_Service.
'
' NOTE:  The service must support pausing and be running already.
'
' NOTE:  This script only applies to NT-based systems since Win9x does support services
'
'
'***************************************************************************
Set ServiceSet = GetObject("winmgmts:").ExecQuery("select * from Win32_Service where Name='Schedule'")

for each Service in ServiceSet
	SupportsPause = Service.AcceptPause
	if SupportsPause = true then
		RetVal = Service.ResumeService()
		if RetVal = 0 then 
			WScript.Echo "Service resumed"   
		else
			if RetVal = 1 then 
				WScript.Echo "Pause not supported" 
			else WScript.Echo "An error occurred:" & RetVal
			End If
		End If
	else
		WScript.Echo "Service does not support pause"
	end if
next
