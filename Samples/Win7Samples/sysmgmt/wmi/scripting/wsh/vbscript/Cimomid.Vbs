' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - CIMOM Identification (VBScript)
'
' This script demonstrates the display of WMI installation info.
'
'***************************************************************************

on error resume next 
set cimomid = GetObject("winmgmts:root\default:__cimomidentification=@")

if err <> 0 then
	WScript.Echo ErrNumber, Err.Source, Err.Description
else
	WScript.Echo cimomid.path_.displayname
	WScript.Echo cimomid.versionusedtocreatedb
end if