' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Schema browsing (VBScript)
'
' This script demonstrates browsing of qualifiers, properties and methods.
'
'***************************************************************************

On Error Resume Next
Set Process = GetObject("winmgmts:Win32_Process")
WScript.Echo ""
WScript.Echo "Class name is", Process.Path_.Class

'Get the properties
WScript.Echo ""
WScript.Echo "Properties:"
WScript.Echo ""
for each Property in Process.Properties_
	WScript.Echo " " & Property.Name
next

'Get the qualifiers
WScript.Echo ""
WScript.Echo "Qualifiers:"
WScript.Echo ""
for each Qualifier in Process.Qualifiers_
	WScript.Echo " " & Qualifier.Name
next

'Get the methods
WScript.Echo ""
WScript.Echo "Methods:"
WScript.Echo ""
for each Method in Process.Methods_
	WScript.Echo " " & Method.Name
next


if Err <> 0 Then
	WScript.Echo Err.Description
	Err.Clear
End if