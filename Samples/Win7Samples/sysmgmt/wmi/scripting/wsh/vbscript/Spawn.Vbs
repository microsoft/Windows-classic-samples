' Copyright (c)  Microsoft Corporation, All Rights Reserved 
'***************************************************************************
' 
' WMI Sample Script - Subclass creation (VBScript)
'
' This script demonstrates how to create a subclass.
'
'***************************************************************************

on error resume next
Set t_Service = GetObject("winmgmts://./root/default")
Set t_Class = t_Service.Get
t_Class.Path_.Class = "MyBaseClass"
t_Class.Put_


Set t_SubClass = t_Service.Get("MyBaseClass").SpawnDerivedClass_
t_Subclass.Path_.Class = "SPAWNCLASSTEST00"

t_Subclass.Put_

if err <> 0 then
	WScript.Echo Err.Number, Err.Description, Err.Source
end if 



