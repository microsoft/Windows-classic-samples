' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - List network protocols (VBScript)
'
' This script demonstrates how to retrieve a list of running services from instances of
' Win32_NetworkProtocol.
'
'***************************************************************************
Set ProtocolSet = GetObject("winmgmts:").ExecQuery("select * from Win32_NetworkProtocol")

for each Protocol in ProtocolSet
	WScript.Echo Protocol.Name
next
