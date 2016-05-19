' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Method invocation (VBScript)
'
' This script demonstrates how to invoke a CIM method
' as if it were an automation method of SWbemObject
'
'***************************************************************************

on error resume next

set process = GetObject("winmgmts:Win32_Process")

result = process.Create ("notepad.exe",null,null,processid)

WScript.Echo "Method returned result = " & result
WScript.Echo "Id of new process is " & processid

if err <>0 then
	WScript.Echo Err.Description, "0x" & Hex(Err.Number)
end if