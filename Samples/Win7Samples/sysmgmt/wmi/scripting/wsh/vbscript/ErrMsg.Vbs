' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Error and error object handling (VBScript)
'
' This script demonstrates how to inspect error and error object
' information.
'
'***************************************************************************
On Error Resume Next

'Ask for non-existent class to force error

Set t_Service = GetObject("winmgmts://./root/default")
Set t_Object = t_Service.Get("Nosuchclass000")

if Err = 0 Then
	WScript.Echo "Got a class"
Else
	WScript.Echo ""
	WScript.Echo "Err Information:"
	WScript.Echo ""
	WScript.Echo "  Source:", Err.Source
	WScript.Echo "  Description:", Err.Description
	WScript.Echo "  Number", "0x" & Hex(Err.Number)

	'Create the last error object
	set t_Object = CreateObject("WbemScripting.SWbemLastError")
	WScript.Echo ""
	WScript.Echo "WMI Last Error Information:"
	WScript.Echo ""
	WScript.Echo " Operation:", t_Object.Operation
	WScript.Echo " Provider:", t_Object.ProviderName

	strDescr = t_Object.Description
	strPInfo = t_Object.ParameterInfo
	strCode = t_Object.StatusCode

	if (strDescr <> nothing) Then
		WScript.Echo " Description:", strDescr		
	end if

	if (strPInfo <> nothing) Then
		WScript.Echo " Parameter Info:", strPInfo		
	end if

	if (strCode <> nothing) Then
		WScript.Echo " Status:", strCode		
	end if

	WScript.Echo ""
	Err.Clear
	set t_Object2 = CreateObject("WbemScripting.SWbemLastError")
	if Err = 0 Then
	   WScript.Echo "Got the error object again - this shouldn't have happened!"	
	Else
	   Err.Clear
	   WScript.Echo "Couldn't get last error again - as expected"
	End if
End If

