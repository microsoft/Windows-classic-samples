' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - View Page File settings (VBScript)
'
' This script demonstrates how to retrieve page file stats from instances of
' Win32_PageFile.
'
'***************************************************************************
Set PageFileSet = GetObject("winmgmts:").InstancesOf ("Win32_PageFile")

for each PageFile in PageFileSet
	WScript.Echo PageFile.Name & Chr(13) 
	WScript.Echo " Initial: " & PageFile.InitialSize & " MB"
	WScript.Echo " Max: " & PageFile.MaximumSize & " MB"	

next
