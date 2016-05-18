' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - SWbemObject.Derivation_ access (VBScript)
'
' This script demonstrates the manipulation of the derivation_ property
' of SWbemObject.
'
'***************************************************************************
on Error resume next

Set c = GetObject("winmgmts://./root/cimv2:win32_logicaldisk")
d = c.Derivation_

for x = LBound(d) to UBound(d)
	WScript.Echo d(x)
Next

if err <> 0 then
	WScript.Echo Err.Description
end if