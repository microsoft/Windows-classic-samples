' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'
' This sample illustrates how to retrieve registry data from WMI.  Note that the accompanying MOF
' (registry.mof) must be compiled and loaded for this sample to run correctly.
'
on error resume next
set transports = GetObject("winmgmts:root/registryScriptExample").InstancesOf ("RegTrans")

for each transport in transports
	WScript.Echo "Transport " & transport.TransportsGUID & " has name [" & transport.Name & "] and Enabled=" _
			& transport.Enabled
next